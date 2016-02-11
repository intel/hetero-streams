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

#include <string>

#include "hStreams_locks.h"
#include "hStreams_LogDomain.h"
#include "hStreams_PhysStreamCOI.h"
#include "hStreams_common.h" // for HSTR_MAX_FUNC_NAME_SIZE
#include "hStreams_internal.h"
#include "hStreams_Logger.h"

hStreams_PhysStreamCOI::hStreams_PhysStreamCOI(
    hStreams_LogDomain &log_dom,
    hStreams_CPUMask const &cpu_mask,
    HSTR_COIPIPELINE coi_pipeline,
    HSTR_COIFUNCTION thunk_func
)
    : hStreams_PhysStream(log_dom, cpu_mask), coi_pipeline_(coi_pipeline), thunk_func_(thunk_func)
{
}

hStreams_PhysStreamCOI::~hStreams_PhysStreamCOI()
{
    // Due to what seems to be a bug in COI, this call to COIPipelineDestroy
    // will have to be protected by a mutex, to serialize them.
    static hStreams_Lock COIPipelineDestroy_lock;
    hStreams_Scope_Locker_Unlocker autolock(COIPipelineDestroy_lock);
    HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIPipelineDestroy(coi_pipeline_);
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC)
                << "A problem encountered while destroying pipeline "
                << hStreams_COIWrapper::COIResultGetName(coi_res);
    }
}

HSTR_RESULT hStreams_PhysStreamCOI::impl_enqueueFunction(
    std::vector<uint64_t> &args,
    std::vector<HSTR_EVENT> &input_deps,
    void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
)
{
    HSTR_COI_ACCESS_FLAGS  *flags_ptr = NULL;
    HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIPipelineRunFunction(
                                 coi_pipeline_,
                                 thunk_func_,
                                 0, NULL,
                                 flags_ptr,
                                 (uint32_t)input_deps.size(),
                                 (input_deps.size()) ? &input_deps[0] : NULL,
                                 &args[0],
                                 (uint16_t)args.size() * sizeof(uint64_t),
                                 ret_val, ret_val_size,
                                 ret_event
                             );
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Couldn't run function on the sink: "
                << hStreams_COIWrapper::COIResultGetName(coi_res);

        return HSTR_RESULT_REMOTE_ERROR;
    }
    return HSTR_RESULT_SUCCESS;
}


