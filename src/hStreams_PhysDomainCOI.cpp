/*
 * Hetero Streams Library - A streaming library for heterogeneous platforms
 * Copyright (c) 2014 - 2016, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */

#include "hStreams_PhysDomainCOI.h"
#include "hStreams_PhysStreamCOI.h"
#include "hStreams_LogDomain.h"
#include "hStreams_internal.h"
#include "hStreams_helpers_source.h"
#include "hStreams_Logger.h"

namespace
{
hStreams_CPUMask generateMaxCPUMask(HSTR_COI_ENGINE_INFO const &coi_eng_info)
{
    hStreams_CPUMask ret;
    uint32_t t, nthr = coi_eng_info.NumThreads;
    for (t = 0; t < nthr; t++) {
        HSTR_CPU_MASK_SET(t, ret.mask);
    }
    return ret;
}

hStreams_CPUMask generateAvoidCPUMask(HSTR_COI_ENGINE_INFO const &coi_eng_info)
{
    hStreams_CPUMask ret;
    uint32_t t, nthr = coi_eng_info.NumThreads;
    for (t = 0; t < nthr; t++) {
        if (t < HSTR_OS_START_ADJUST || t >= (nthr - HSTR_OS_END_ADJUST)) {
            HSTR_CPU_MASK_SET(t, ret.mask);
        }
    }
    return ret;
}
} // anonymous namespace

hStreams_PhysDomainCOI::hStreams_PhysDomainCOI(
    HSTR_PHYS_DOM id,
    HSTR_COIPROCESS coi_process,
    HSTR_COI_ENGINE_INFO const &coi_eng_info,
    std::vector<HSTR_COILIBRARY> const &sink_libs,
    HSTR_COIFUNCTION thunk_func,
    HSTR_COIFUNCTION fetch_addr_func,
    HSTR_COIPIPELINE helper_pipeline
)
    :
    hStreams_PhysDomain(
       id,
       coi_eng_info.ISA,
       coi_eng_info.CoreMaxFrequency,
       coi_eng_info.PhysicalMemory,
       generateMaxCPUMask(coi_eng_info),
       generateAvoidCPUMask(coi_eng_info)
    ),
    coi_process_(coi_process),
    sink_libs_(sink_libs),
    thunk_func_(thunk_func),
    fetch_addr_func_(fetch_addr_func),
    helper_pipeline_(helper_pipeline)
{
}

hStreams_PhysDomainCOI::~hStreams_PhysDomainCOI()
{
    HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIPipelineDestroy(helper_pipeline_);
    // Result checking
    if (coi_res != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "A problem has been encountered while destroying pipeline: "
                << hStreams_COIWrapper::COIResultGetName(coi_res);
    }

    for (SinkLibsContainer::iterator it = sink_libs_.begin(); it != sink_libs_.end(); ++it) {
        HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIProcessUnloadLibrary(coi_process_, *it);
        if (HSTR_COI_SUCCESS != coi_res) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "A problem has been encountered while unloading a sink-side library: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);
        }
    }
    coi_res = hStreams_COIWrapper::COIProcessDestroy(coi_process_, -1, 0, NULL, NULL);
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "A problem has been encountered while destroying a sink-side process: "
                << hStreams_COIWrapper::COIResultGetName(coi_res);

    }
}

hStreams_PhysStream *hStreams_PhysDomainCOI::impl_createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask)
{
    HSTR_COIPIPELINE coi_pipeline;
    HSTR_CPU_MASK pipeline_mask;
    memcpy(pipeline_mask, cpu_mask.mask, sizeof(HSTR_CPU_MASK));
    HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIPipelineCreate(
                                 getCOIProcess(),// Process to associate the pipeline with
                                 pipeline_mask, // Set sink thread affinity for the pipeline
                                 0,                  // Use the default stack size for the pipeline thread
                                 &coi_pipeline       // Handle to the new pipeline
                             );
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Couldn't spawn off a physical stream: " << hStreams_COIWrapper::COIResultGetName(coi_res);

        return NULL;
    }

    hStreams_PhysStream *phys_stream = new hStreams_PhysStreamCOI(log_dom, cpu_mask, coi_pipeline, thunk_func_);

    if (!phys_stream) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Internal error while creating the physical stream.";

        return NULL;
    }

    uint64_t num = (uint64_t)(HSTR_CPU_MASK_COUNT(getMaxCPUMask().mask) -
                              HSTR_CPU_MASK_COUNT(getAvoidCPUMask().mask));

    // Set up affinitized OpenMP on this stream, based on this streams CPUmask
    if (hStreams_GetOptions_openmp_policy() == HSTR_OPENMP_PRE_SETUP) {
        HSTR_RESULT hret = hStreams_helper_func_19parm(
                               *phys_stream,
                               "hStreams_init_partition",
                               (uint64_t)id(),
                               (uint64_t)42, /* logical stream ID, used only for printing on sink FIXME?*/
                               num,
                               cpu_mask.mask[0],
                               cpu_mask.mask[1],
                               cpu_mask.mask[2],
                               cpu_mask.mask[3],
                               cpu_mask.mask[4],
                               cpu_mask.mask[5],
                               cpu_mask.mask[6],
                               cpu_mask.mask[7],
                               cpu_mask.mask[8],
                               cpu_mask.mask[9],
                               cpu_mask.mask[10],
                               cpu_mask.mask[11],
                               cpu_mask.mask[12],
                               cpu_mask.mask[13],
                               cpu_mask.mask[14],
                               cpu_mask.mask[15]);
        if (HSTR_RESULT_SUCCESS != hret) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "An error was encountered while setting up OpenMP on the sink: "
                    << hStreams_ResultGetName(hret);

            delete phys_stream;
            return NULL;
        }
    }
    return phys_stream;
}

uint64_t hStreams_PhysDomainCOI::impl_fetchSinkFunctionAddress(std::string const &func_name)
{
    void *sink_func_addr = NULL;
    HSTR_COI_ACCESS_FLAGS  *fetch_flags_ptr = NULL;
    HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIPipelineRunFunction(helper_pipeline_, fetch_addr_func_,
                             0, NULL, fetch_flags_ptr, 0, NULL, func_name.c_str(),
                             (int16_t) func_name.size() + 1, (void *)&sink_func_addr,
                             (uint16_t) sizeof(sink_func_addr), NULL);
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Couldn't get sink-side function address: " << hStreams_COIWrapper::COIResultGetName(coi_res);

        return 0;
    }
    return (uint64_t)sink_func_addr;
}
