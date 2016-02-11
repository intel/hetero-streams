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

#include <hStreams_internal.h>
#include <hStreams_atomic.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <dlfcn.h>
#include <unistd.h>
#include <tr1/unordered_set>
#else
#include <unordered_set>
#endif
#include "hStreams_internal_vars_source.h"
#include "hStreams_PhysDomainCollection.h"
#include "hStreams_PhysDomainCOI.h"
#include "hStreams_PhysDomainHost.h"
#include "hStreams_LogDomain.h"
#include "hStreams_LogStream.h"
#include "hStreams_LogBuffer.h"
#include "hStreams_COIWrapper.h"
#include "hStreams_helpers_source.h"
#include "hStreams_exceptions.h"
#include "hStreams_Logger.h"
#include "hStreams_internal_types_common.h"

#ifdef __cplusplus
extern "C" {
#endif


static HSTR_RESULT
hStreams_Init_worker(int hstreams_library_version)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(hstreams_library_version);

    if (hstreams_library_version != HSTR_SECOND_PREVIEW_LIBRARY_VERSION) {

        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "hStreams_Init_worker was called with an unsupported hStreams library version: "
                << hstreams_library_version;

        // unless _hStreams_FatalError is overridden, we should never get here
        return HSTR_RESULT_INTERNAL_ERROR;
    }

    // NOTE: the following declaration introduces a critical section of code.
    // While the local variable: 'initLock' is in scope, the code will serialize
    // all threads of execution.  The net effect is hStreams_Init will always
    // operate in a single thread of execution.
    hStreams_Scope_Locker_Unlocker initLock(hstr_proc.hStreamsInitLock);

    if (hStreams_IsInitialized() == HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_SUCCESS;
    }

    hstr_proc.hStreamsState.SetValue(HSTR_STATE_INITIALIZING);

    // Constructor will load COI library if it isn't loaded
    new hStreams_COIWrapper;

    //Clear any error if previously set.
    hStreams_ClearLastError();

    assert(0 == (((uint64_t)&hstr_proc.dummy_buf) & 63));
    assert(0 == (((uint64_t)&hstr_proc.dummy_data) & 63));

    uint32_t            i, active_domains, first_valid_domainID = (uint32_t) - 1;
    const char         *thunk_name                  = "hStreamsThunk";
    const char         *fetchSinkFuncAddress_name   = "hStreams_fetchSinkFuncAddress";
    string              executableFileName;
    HSTR_RESULT         hsr;
    HSTR_COIRESULT           result;
    HSTR_COIPROCESS dummy_process = NULL;

    if ((hsr = hStreams_FetchExecutableName(executableFileName)) != HSTR_RESULT_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot obtain executable name.";

        executableFileName.assign("");
    } else {
        HSTR_LOG(HSTR_INFO_TYPE_MISC) << "Executable file name: '" << executableFileName << "'.";
    }

    hStreams_ClearLastError();

    // May return HSTR_COI_DOES_NOT_EXIST if COI_ISA_MIC is not matched
    result = hStreams_COIWrapper::COIEngineGetCount(HSTR_ISA_MIC, &hstr_proc.myNumPhysDomains);
    // Result checking
    if (result != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "COIEngineGetCount returned " << hStreams_COIWrapper::COIResultGetName(result);

        HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
    }


    // Artificially constrain the number of domains
    // FIXME: Currently it uses the lowest-indexed domains
    if (hstr_proc.myNumPhysDomains > hStreams_GetOptions_phys_domains_limit()) {
        hstr_proc.myNumPhysDomains = hStreams_GetOptions_phys_domains_limit();
    }

    // REVIEW: Assumptions
    //  myNumPhysDomains is based on SCIF enumeration, regardless of whether
    //  domains are currently active, and it's then constrained by the user.
    //  COIEngineGetHandle accurately reports HSTR_COI_NOT_INITIALIZED, whether
    //  domains go offline before or after COI initialization

    for (i = 0, active_domains = 0; i < hstr_proc.myNumPhysDomains; i++) {

        // FIXME: Currently we handle only MIC domains, we need to handle x86_64 domains also.
        //        Those need to include the special case of a host domain, where there's no COI daemon

        // can become invalid since COI was initialized for this proc; not incrementally maintained; very small window
        // advance through any domains that became invalid after SCIF enumeration
        HSTR_COIRESULT coi_res;
        HSTR_COIENGINE coi_eng;

        coi_res = hStreams_COIWrapper::COIEngineGetHandle(HSTR_ISA_MIC, i, &coi_eng);
        if (HSTR_COI_SUCCESS != coi_res) {
            continue;
        }

        HSTR_COI_ENGINE_INFO coi_eng_info;
        coi_res = hStreams_COIWrapper::COIEngineGetInfo(coi_eng, sizeof(HSTR_COI_ENGINE_INFO), &coi_eng_info);
        if (HSTR_COI_SUCCESS != coi_res) {
            continue;
        }

        HSTR_COIPROCESS coi_process;
        coi_res = hStreams_COIWrapper::COIProcessCreateFromMemory(coi_eng, "KNC_startup",
                  (void *)KNC_startup, KNC_startup_size,
                  0, NULL, false, NULL, true, NULL, 1024 * 1024, NULL,
                  NULL, 0, &coi_process);
        if (HSTR_COI_SUCCESS != coi_res) {
            hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Could not create process on the device: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);

            // unless _hStreams_FatalError is overridden, we should never get here
            return HSTR_RESULT_REMOTE_ERROR;
        }
        std::vector<HSTR_COILIBRARY> loaded_libs;
        CHECK_HSTR_RESULT(hStreams_LoadSinkSideLibrariesMIC(coi_process, loaded_libs, executableFileName));

        // Get the handle for the thunk, thunk_func
        HSTR_COIFUNCTION thunk_func;
        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &thunk_name, &thunk_func);
        if (coi_res != HSTR_COI_SUCCESS) {
            hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
            // FIXME: It seems at this point that we need to do some finalization of the
            // initialization that has happened to this point?
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Sink-side library does not contain a function named: " << thunk_name
                    << ", result: " << hStreams_COIWrapper::COIResultGetName(coi_res);

            // unless _hStreams_FatalError is overridden, we should never get here
            return HSTR_RESULT_BAD_NAME;
        }

        // Get the handle for the fetch sink func address function
        HSTR_COIFUNCTION fetch_addr_func;
        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &fetchSinkFuncAddress_name,
                  &fetch_addr_func);
        if (coi_res != HSTR_COI_SUCCESS) {
            hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
            // FIXME: It seems at this point that we need to do some finalization of the
            // initialization that has happened to this point?
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Sink-side library does not contain a function named: " << fetchSinkFuncAddress_name
                    << ", result: " << hStreams_COIWrapper::COIResultGetName(coi_res);

            // unless _hStreams_FatalError is overridden, we should never get here
            return HSTR_RESULT_BAD_NAME;
        }
        // The helper pipeline will be first used to initialize the sink side
        // of things by calling hStreams_init_sink.
        // Later, it will be passed to the hStreams_PhysDomainCOI's constructor and
        // it takes the ownership of this helper pipeline and will be reponsible for
        // destroying it.
        HSTR_COIPIPELINE helper_coi_pipeline;
        coi_res = hStreams_COIWrapper::COIPipelineCreate(coi_process, 0, 0, &helper_coi_pipeline);
        // Result checking
        if (coi_res != HSTR_COI_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "A problem encountered while creating pipeline: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);

            HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
        }

        HSTR_COIFUNCTION init_sink;
        const char *func_name_init_sink =     "hStreams_init_sink";

        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &func_name_init_sink,
                  &init_sink);
        if (HSTR_COI_SUCCESS != coi_res) {
            hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
            // FIXME: It seems at this point that we need to do some finalization of the
            // initialization that has happened to this point?
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Sink-side library does not contain a function named: " << func_name_init_sink
                    << ", result: " << hStreams_COIWrapper::COIResultGetName(coi_res);

            return HSTR_RESULT_BAD_NAME;
        }

        hStreams_InitSinkData init_data;

        hStreams_GetCurrentOptions(&(init_data.options), sizeof(init_data.options));
        init_data.phys_domain_id = active_domains;
        init_data.logging_bitmask = globals::logging_bitmask;
        init_data.logging_level = globals::logging_level;
        init_data.logging_myphysdom = globals::logging_myphysdom;
        init_data.mkl_interface = globals::mkl_interface;

        uint64_t error_code_buf = HSTR_RESULT_SUCCESS;

        coi_res = hStreams_COIWrapper::COIPipelineRunFunction(helper_coi_pipeline, init_sink, 0, NULL, NULL, 0,
                  NULL, &init_data, sizeof(init_data),
                  (void *)&error_code_buf, sizeof(error_code_buf), NULL);
        // Result checking
        if (coi_res != HSTR_COI_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "A problem encountered while running a function in the pipeline: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);

            HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
        }

        HSTR_RESULT error_code = (HSTR_RESULT)error_code_buf;

        if (error_code != HSTR_RESULT_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Could not initialize physical domain " << active_domains
                    << ". Remote process indicated the error code to be: "
                    << error_code << ", i.e. " << hStreams_ResultGetName(error_code);

            HSTR_RETURN(error_code);
        }

        hStreams_PhysDomain *phys_dom = new hStreams_PhysDomainCOI(active_domains, coi_process,
                coi_eng_info, loaded_libs, thunk_func, fetch_addr_func, helper_coi_pipeline);

        phys_domains.addToCollection(phys_dom);

        if (first_valid_domainID == (uint32_t) - 1) {
            first_valid_domainID = i;
            // Save some dummy COI process to be used by
            // COIBufferCreateFromMemory on the source physical domain
            dummy_process = coi_process;
        }
        active_domains++; // Advance valid engines
    }

    std::vector<LIB_HANDLER::handle_t> loaded_libs_handles;
    hStreams_LoadSinkSideLibrariesHost(executableFileName, loaded_libs_handles);

    hStreams_PhysDomain *host_phys_dom = new hStreams_PhysDomainHost(dummy_process, loaded_libs_handles);
    phys_domains.addToCollection(host_phys_dom);
    HSTR_CPU_MASK dummy_mask;
    HSTR_CPU_MASK_ZERO(dummy_mask);
    hStreams_LogDomain *source_log_dom = new hStreams_LogDomain(HSTR_SRC_LOG_DOMAIN, dummy_mask, *host_phys_dom);
    log_domains.addToCollection(source_log_dom);


    if (active_domains == 0) {
        // FIXME: It seems at this point that we need to do some finalization of the
        // initialization that has happened to this point?  Or, are we depending on
        // exit() cleaning up after us?
        hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "No MIC cards in system.  Use of hStreams is not permitted.";

        // unless _hStreams_FatalError is overridden, we should never get here
        return HSTR_RESULT_DEVICE_NOT_INITIALIZED;
    }

    // Check that all physical domains have the same resources
    hstr_proc.homogeneous = phys_domains.isHomogenous();

    // Create dummy buffer for EventStreamWait
    result = hStreams_COIWrapper::COIBufferCreate(
                 1,                             // Size: 1. Smallest performant size
                 HSTR_COI_BUFFER_OPENCL,             // COI Buffer Type: OCL, non-thread specific
                 0,                             // Flags: 0
                 NULL,                          // not initialized
                 1, &dummy_process, // Unused but required
                 &hstr_proc.dummy_buf);        // Assign to pointer to COIBuffer

    // Result checking
    if (result != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "A problem encountered while creating buffer: "
                << hStreams_COIWrapper::COIResultGetName(result);

        HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
    }

    // Check envirables for 2M buffer usage.  DMA is faster if host and device
    //   use 2MB.
    // Host usage of 2MB requires either
    //  THP, enabled on RH Linux if
    //   /sys/kernel/mm/redhat_transparent_hugepage/enable is always
    //  User forces 2MB with mmap
#define STRLEN_2MB 40
    char env_2MB[STRLEN_2MB + 1];
    const char *const env_2MB_var = getenv("MIC_USE_2MB_BUFFERS");
    if (env_2MB_var && strlen(env_2MB_var)) {
        strncpy(env_2MB, env_2MB_var, STRLEN_2MB);
        env_2MB[STRLEN_2MB] = 0;
        env_2MB[STRLEN_2MB - 1] = '\0'; // be safe in case length exceeded
        // Format is <int>[BKMGT]
        uint64_t multiplier = 1;
        if (strlen(env_2MB) > 0) { // This second strlen check is not needed but is required due to Kkloc work.
            switch (env_2MB[strlen(env_2MB) - 1]) { // size
            case '\0':
                break;
            case 'T':
            case 't':
                multiplier *= 1024;
            case 'G':
            case 'g':
                multiplier *= 1024;
            case 'M':
            case 'm':
                multiplier *= 1024;
            case 'K':
            case 'k':
                multiplier *= 1024;
            case 'B':
            case 'b':
                // strip the size suffix, if present
                env_2MB[strlen(env_2MB) - 1] = '\0';
                break;
            default:
                HSTR_WARN(HSTR_INFO_TYPE_MISC)
                        << env_2MB[strlen(env_2MB) - 1]
                        << " is not a valid suffix for MIC_USE_2MB_BUFFERS, please use one of [BKMGT] or none.";
            }
            huge_page_usage_threshold = atoi(env_2MB) * multiplier;
        }
    }

    hstr_proc.hStreamsState.SetValue(HSTR_STATE_INITIALIZED);
    hstr_proc.myActivePhysDomains = active_domains;

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Init , 1)()
{
    try {
        HSTR_TRACE_FUN_ENTER();

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
        HSTR_RETURN(hStreams_Init_worker(HSTR_SECOND_PREVIEW_LIBRARY_VERSION));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_IsInitialized , 1)()
{
    try {
        HSTR_TRACE_FUN_ENTER();

        return (hstr_proc.hStreamsState.GetValue() == HSTR_STATE_INITIALIZED) ?
               HSTR_RESULT_SUCCESS  :
               HSTR_RESULT_NOT_INITIALIZED;
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

// Forward declaration
static HSTR_RESULT
hStreams_StreamDestroy_worker(HSTR_LOG_STR    in_LogStreamID);

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Fini , 1)()
{
    try {
        HSTR_TRACE_FUN_ENTER();

        // NOTE: the following declaration introduces a critical section of code.
        // While the local variable: 'finiLock' is in scope, the code will serialize
        // all threads of execution.  The net effect is hStreams_Fini will always
        // operate in a single thread of execution.
        hStreams_Scope_Locker_Unlocker finiLock(hstr_proc.hStreamsFiniLock);

        CHECK_HSTR_RESULT(hStreams_IsInitialized());

        // The following early return is to allow a thread of execution that squeezed in a
        // race to be the one thread to execute the actual fini code but it actually lost the race.
        if (hstr_proc.hStreamsState.GetValue() == HSTR_STATE_UNINITIALIZED) {
            HSTR_RETURN(HSTR_RESULT_SUCCESS);
        }

        hstr_proc.hStreamsState.SetValue(HSTR_STATE_FINALIZING);

        // Wait until all threads of execution that are executing in a thread of execution
        // in any call of the hStreams library:
        while (hstr_proc.callCounter.GetValue() > 0) {
            hStreams_SleepMS(1);
        }

        HSTR_COIRESULT result = hStreams_COIWrapper::COIBufferDestroy(hstr_proc.dummy_buf);
        // Result checking
        if (result != HSTR_COI_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                    << "A problem encountered while destroying buffer: "
                    << hStreams_COIWrapper::COIResultGetName(result);

            HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
        }

        log_buffers.destroyAllBuffers();
        log_streams.destroyAllStreams();
        log_domains.destroyAllDomains();
        phys_domains.destroyAllDomains();

        hStreams_ResetDefaultOptions();

        hstr_proc.hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
        hstr_proc.myActivePhysDomains = 0;
        globals::app_init_next_log_str_ID = globals::initial_values::app_init_next_log_str_ID;
        next_log_dom_id = globals::initial_values::next_log_dom_id;
        globals::mkl_interface = globals::initial_values::mkl_interface;
        globals::app_init_log_doms_IDs.clear();
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetNumPhysDomains , 1)(
    uint32_t          *out_pNumPhysDomains,
    uint32_t          *out_pNumActivePhysDomains,
    bool              *out_pHomogeneous)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(out_pNumPhysDomains);
        HSTR_TRACE_FUN_ARG(out_pNumActivePhysDomains);
        HSTR_TRACE_FUN_ARG(out_pHomogeneous);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }
        if (out_pNumPhysDomains        == NULL ||
                out_pNumActivePhysDomains  == NULL ||
                out_pHomogeneous           == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }
        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        *out_pNumPhysDomains       = hstr_proc.myNumPhysDomains;
        *out_pNumActivePhysDomains = hstr_proc.myActivePhysDomains;
        *out_pHomogeneous =          hstr_proc.homogeneous;

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT
hStreams_GetPhysDomainDetails_worker(
    int32_t             hstreams_library_version,
    HSTR_PHYS_DOM       in_PhysDomainID,
    uint32_t           *out_pNumThreads,
    HSTR_ISA_TYPE      *out_pISA,
    uint32_t           *out_pCoreMaxMHz,
    HSTR_CPU_MASK       out_MaxCPUmask,
    HSTR_CPU_MASK       out_AvoidCPUmask,
    uint64_t           *out_pSupportedMemTypes,
    uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE])
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(hstreams_library_version);
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);

    if (hstreams_library_version != HSTR_SECOND_PREVIEW_LIBRARY_VERSION) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "GetPhysDomainDetails was called with an unsupported hStreams library version: "
                << hstreams_library_version;

        // unless _hStreams_FatalError is overridden, we should never get here
        return HSTR_RESULT_INTERNAL_ERROR;
    }

    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }

    if ((out_pNumThreads                  == NULL) ||
            (out_pISA                         == NULL) ||
            (out_pCoreMaxMHz                  == NULL) ||
            (out_pSupportedMemTypes           == NULL) ||
            (out_pPhysicalBytesPerMemType     == NULL)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "An output pointer operand of hStreams_GetPhysDomainDetails was NULL";

        return HSTR_RESULT_NULL_PTR;
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == dom) {
        return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
    }

    hStreams_CPUMask max_mask = dom->getMaxCPUMask();
    hStreams_CPUMask avd_mask = dom->getAvoidCPUMask();

    *out_pNumThreads = HSTR_CPU_MASK_COUNT(max_mask.mask);
    memcpy(out_MaxCPUmask, max_mask.mask, sizeof(HSTR_CPU_MASK));
    memcpy(out_AvoidCPUmask, avd_mask.mask, sizeof(HSTR_CPU_MASK));

    *out_pSupportedMemTypes = 0;
    *out_pSupportedMemTypes |= (1 << HSTR_MEM_TYPE_NORMAL);
    // Assure that all valid values get set
    memset(out_pPhysicalBytesPerMemType, 0, HSTR_MEM_TYPE_SIZE * sizeof(uint64_t));
    out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_NORMAL] = dom->available_memory;
    out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_HBW] = 0; // until MCDRAM added in KNL

    *out_pISA = dom->isa;
    *out_pCoreMaxMHz = dom->core_max_freq_MHz;

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetPhysDomainDetails , 1)(
    HSTR_PHYS_DOM       in_PhysDomain,
    uint32_t           *out_pNumThreads,
    HSTR_ISA_TYPE      *out_pISA,
    uint32_t           *out_pCoreMaxMHz,
    HSTR_CPU_MASK       out_MaxCPUmask,
    HSTR_CPU_MASK       out_AvoidCPUmask,
    uint64_t           *out_pSupportedMemTypes,
    uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE])
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomain);
        HSTR_TRACE_FUN_ARG(out_pNumThreads);
        HSTR_TRACE_FUN_ARG(out_pISA);
        HSTR_TRACE_FUN_ARG(out_pCoreMaxMHz);
        HSTR_TRACE_FUN_ARG(out_MaxCPUmask);
        HSTR_TRACE_FUN_ARG(out_AvoidCPUmask);
        HSTR_TRACE_FUN_ARG(out_pSupportedMemTypes);
        HSTR_TRACE_FUN_ARG(out_pPhysicalBytesPerMemType);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        HSTR_RETURN(hStreams_GetPhysDomainDetails_worker(
                        HSTR_SECOND_PREVIEW_LIBRARY_VERSION,
                        in_PhysDomain,
                        out_pNumThreads,
                        out_pISA,
                        out_pCoreMaxMHz,
                        out_MaxCPUmask,
                        out_AvoidCPUmask,
                        out_pSupportedMemTypes,
                        out_pPhysicalBytesPerMemType));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetOversubscriptionLevel, 1)(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumThreads,
    uint32_t       *out_pOversubscriptionArray)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomainID);
        HSTR_TRACE_FUN_ARG(in_NumThreads);
        HSTR_TRACE_FUN_ARG(out_pOversubscriptionArray);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }
        if (out_pOversubscriptionArray == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
        if (NULL == phys_dom) {
            HSTR_RETURN(HSTR_RESULT_DOMAIN_OUT_OF_RANGE);
        }

        if (phys_dom->getNumThreads() != in_NumThreads) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        phys_dom->getOversubscriptionLevel(out_pOversubscriptionArray);

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetAvailable , 1)(
    HSTR_PHYS_DOM       in_PhysDomainID,
    HSTR_CPU_MASK       out_AvailableCPUmask)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomainID);
        HSTR_TRACE_FUN_ARG(out_AvailableCPUmask);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_PhysDomain *dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
        if (NULL == dom) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }

        hStreams_CPUMask ret = dom->getAvailableStreamCPUMask();
        memcpy(out_AvailableCPUmask, ret.mask, sizeof(HSTR_CPU_MASK));

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


static HSTR_RESULT
hStreams_AddLogDomain_worker(
    HSTR_PHYS_DOM      in_PhysDomainID,
    HSTR_CPU_MASK      in_CPUmask,
    HSTR_LOG_DOM      *out_pLogDomainID,
    HSTR_OVERLAP_TYPE *out_pOverlap)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(in_CPUmask);

#ifdef _WIN32
    if (in_PhysDomainID == HSTR_SRC_PHYS_DOMAIN) {
        uint32_t            NumThreads, CoreMaxMHz;
        uint64_t            SupportedMemTypes, PhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE];
        HSTR_ISA_TYPE       ISA;
        HSTR_CPU_MASK       MaxCPUmask, AvoidCPUmask;

        hStreams_GetPhysDomainDetails(
            HSTR_SRC_PHYS_DOMAIN,
            &NumThreads,
            &ISA,
            &CoreMaxMHz,
            MaxCPUmask,
            AvoidCPUmask,
            &SupportedMemTypes,
            PhysicalBytesPerMemType);
        if (NumThreads > 64) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Use of the Windows-based host physical domain which "
                    << "contains more than 64 HW threads is not supported.";

            return HSTR_RESULT_NOT_IMPLEMENTED;
        }

    }
#endif
    // Also, call processDelLogDomain on the buffer collection so that appropriate
    // buffers are created
    if (out_pLogDomainID  == NULL ||
            out_pOverlap  == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }

    if (0 == HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        return HSTR_RESULT_CPU_MASK_OUT_OF_RANGE;
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE); // So that nobody adds new buffers

    hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == phys_dom) {
        return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
    }


    // check the CPU mask - whether it fits in the physical domain's mask
    HSTR_CPU_MASK tmp_cpu_mask;
    HSTR_CPU_MASK_AND(tmp_cpu_mask, phys_dom->getMaxCPUMask().mask, in_CPUmask);


    if (HSTR_CPU_MASK_COUNT(tmp_cpu_mask) < HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        return HSTR_RESULT_CPU_MASK_OUT_OF_RANGE;
    }

    // check the CPU mask - whether it partially overlaps with some other logical domain
    HSTR_OVERLAP_TYPE overlap;
    phys_dom->lookupLogDomainByCPUMask(in_CPUmask, &overlap);
    if (HSTR_PARTIAL_OVERLAP == overlap) {
        *out_pOverlap = overlap;
        return HSTR_RESULT_OVERLAPPING_RESOURCES;
    }

    const HSTR_LOG_DOM newID = getNextLogDomID();
    hStreams_LogDomain *log_dom = new hStreams_LogDomain(newID, in_CPUmask, *phys_dom);


    // create physical buffer for each incremental logical buffer
    hStreams_LogBufferCollection::iterator it;
    for (it = log_buffers.begin(); it != log_buffers.end(); ++it) {
        hStreams_LogBuffer *log_buf = *it;
        if (log_buf->isPropertyFlagSet(HSTR_BUF_PROP_INCREMENTAL)) {
            HSTR_RESULT hret = log_buf->attachExistingLogDomain(*log_dom);
            if (hret != HSTR_RESULT_SUCCESS) {
                HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                        << "Could not instantiate incremental buffer " << log_buf->getStart()
                        << " for new logical domain " << log_dom->id() << ": "
                        << hStreams_ResultGetName(hret);

                // Revert changes if any error occurred
                log_buffers.processDelLogDomain(*log_dom);
                delete log_dom;
                return hret;
            }
        }
    }

    phys_dom->addLogDomainMapping(*log_dom);
    log_domains.addToCollection(log_dom);

    *out_pOverlap = overlap;
    *out_pLogDomainID = newID;
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_AddLogDomain , 1)(
    HSTR_PHYS_DOM      in_PhysDomainID,
    HSTR_CPU_MASK      in_CPUmask,
    HSTR_LOG_DOM      *out_pLogDomainID,
    HSTR_OVERLAP_TYPE *out_pOverlap)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomainID);
        HSTR_TRACE_FUN_ARG(in_CPUmask);
        HSTR_TRACE_FUN_ARG(out_pLogDomainID);
        HSTR_TRACE_FUN_ARG(out_pOverlap);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        HSTR_RETURN(hStreams_AddLogDomain_worker(in_PhysDomainID, in_CPUmask, out_pLogDomainID, out_pOverlap));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT
hStreams_RmLogDomains_worker(
    uint32_t       in_NumLogDomains,
    HSTR_LOG_DOM  *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);

    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
    }
    if (in_pLogDomainIDs == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    // yet to be done.
    for (uint32_t log_dom_idx = 0; log_dom_idx < in_NumLogDomains; ++log_dom_idx) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[log_dom_idx]);
        if (NULL == log_dom) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Logical domain with ID in_pLogDomainIDs[" << log_dom_idx
                    << "] == " << in_pLogDomainIDs[log_dom_idx] << " doesn't exist.";

            return HSTR_RESULT_NOT_FOUND;
        }
        log_streams.delFromCollectionByLogDomain(*log_dom);
        log_dom->destroyAllStreams();
        // Finally, erase the logical domain form the mappings and destroy it
        log_domains.delFromCollection(*log_dom);
        log_dom->getPhysDomain().delLogDomainMapping(*log_dom);
        log_buffers.processDelLogDomain(*log_dom);
        delete log_dom;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_RmLogDomains , 1)(
    uint32_t       in_NumLogDomains,
    HSTR_LOG_DOM  *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_NumLogDomains);
        HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);

        hStreams_ScopedAtomicCounter  sac(hstr_proc.callCounter);
        HSTR_RETURN(hStreams_RmLogDomains_worker(in_NumLogDomains, in_pLogDomainIDs));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetNumLogDomains , 1)(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t       *out_pNumLogDomains)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomainID);
        HSTR_TRACE_FUN_ARG(out_pNumLogDomains);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        if (out_pNumLogDomains    == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
        if (NULL == phys_dom) {
            HSTR_RETURN(HSTR_RESULT_DOMAIN_OUT_OF_RANGE);
        }

        *out_pNumLogDomains = phys_dom->getNumLogDomains();

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetLogDomainIDList , 1)(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumLogDomains,
    HSTR_LOG_DOM   *out_pLogDomainIDs)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_PhysDomainID);
        HSTR_TRACE_FUN_ARG(in_NumLogDomains);
        HSTR_TRACE_FUN_ARG(out_pLogDomainIDs);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }
        if (out_pLogDomainIDs    == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
        if (NULL == phys_dom) {
            HSTR_RETURN(HSTR_RESULT_DOMAIN_OUT_OF_RANGE);
        }

        uint32_t num_written, num_present;
        phys_dom->getLogDomainIDs(out_pLogDomainIDs, in_NumLogDomains, &num_written, &num_present);

        if (num_written < in_NumLogDomains) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        if (num_present > in_NumLogDomains) {
            HSTR_RETURN(HSTR_RESULT_INCONSISTENT_ARGS);
        }

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetLogDomainDetails , 1)(
    HSTR_LOG_DOM   in_LogDomainID,
    HSTR_PHYS_DOM *out_pPhysDomainID,
    HSTR_CPU_MASK  out_CPUmask)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogDomainID);
        HSTR_TRACE_FUN_ARG(out_pPhysDomainID);
        HSTR_TRACE_FUN_ARG(out_CPUmask);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        if (out_pPhysDomainID    == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        } // no point in checking out_CPUmask since it's an array, not a pointer

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);
        if (NULL == log_dom) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        memcpy(out_CPUmask, log_dom->getCPUMask().mask, sizeof(HSTR_CPU_MASK));
        *out_pPhysDomainID = log_dom->getPhysDomain().id();

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_StreamCreate , 1)(
    HSTR_LOG_STR        in_LogStreamID,
    HSTR_LOG_DOM        in_LogDomainID,
    const HSTR_CPU_MASK in_CPUmask)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_LogDomainID);
        HSTR_TRACE_FUN_ARG(in_CPUmask);

        hStreams_ScopedAtomicCounter  sac(hstr_proc.callCounter);
        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        if (0 == HSTR_CPU_MASK_COUNT(in_CPUmask)) {
            HSTR_RETURN(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

        hStreams_LogStream *lookup_str = log_streams.lookupByLogStreamID(in_LogStreamID);
        if (NULL != lookup_str) {
            HSTR_RETURN(HSTR_RESULT_ALREADY_FOUND);
        }

        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);
        if (NULL == log_dom) {
            HSTR_RETURN(HSTR_RESULT_DOMAIN_OUT_OF_RANGE);
        }

        // check the CPU mask - whether it fits in the logical domain's mask
        HSTR_CPU_MASK tmp_cpu_mask;
        HSTR_CPU_MASK_AND(tmp_cpu_mask, log_dom->getCPUMask().mask, in_CPUmask);
        if (HSTR_CPU_MASK_COUNT(tmp_cpu_mask) < HSTR_CPU_MASK_COUNT(in_CPUmask)) {
            HSTR_RETURN(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE);
        }

        // Because for some reason, this special API, StreamCreate takes a const CPU mask... why?!
        HSTR_CPU_MASK non_const_cpu_mask;
        memcpy(non_const_cpu_mask, in_CPUmask, sizeof(HSTR_CPU_MASK));


        hStreams_LogStream *new_log_stream = NULL;
        // check the CPU mask - whether it overlaps with some other logical domain
        HSTR_OVERLAP_TYPE overlap;
        hStreams_LogStream *other_log_str = log_dom->lookupLogStreamByCPUMask(non_const_cpu_mask, &overlap);
        if (HSTR_EXACT_OVERLAP == overlap && NULL != other_log_str) {
            // just attach to the logical stream's physical stream
            new_log_stream = new hStreams_LogStream(in_LogStreamID, non_const_cpu_mask, *log_dom, other_log_str->getPhysStream());
        } else {
            // Otherwise, we have to create a new stream
            hStreams_PhysStream *new_phys_stream = log_dom->getPhysDomain().createNewPhysStream(*log_dom, non_const_cpu_mask);
            if (!new_phys_stream) {
                HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
            }
            new_log_stream = new hStreams_LogStream(in_LogStreamID, non_const_cpu_mask, *log_dom, *new_phys_stream);
            // log stream has already attached to the physical stream by itself, we can detach now
            // must be done after hStreams_LogStream initialization with new.
            new_phys_stream->detach();
        }
        if (!new_log_stream) {
            HSTR_RETURN(HSTR_RESULT_INTERNAL_ERROR);
        }

        log_streams.addToCollection(new_log_stream);
        log_dom->addLogStreamMapping(*new_log_stream);

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT
hStreams_StreamDestroy_worker(HSTR_LOG_STR    in_LogStreamID)
{
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    hStreams_LogStream *log_str = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_str) {
        return HSTR_RESULT_NOT_FOUND;
    }

    log_streams.delFromCollection(*log_str);
    log_str->getLogDomain().delLogStreamMapping(*log_str);
    delete log_str;
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_StreamDestroy , 1)(
    HSTR_LOG_STR    in_LogStreamID)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        HSTR_RETURN(hStreams_StreamDestroy_worker(in_LogStreamID));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
hStreams_GetNumLogStreams(
    HSTR_LOG_DOM   in_LogDomainID,
    uint32_t      *out_pNumLogStreams)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogDomainID);
        HSTR_TRACE_FUN_ARG(out_pNumLogStreams);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }
        if (out_pNumLogStreams == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        //What with exposed_deps ?
        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);

        if (NULL == log_dom) {
            HSTR_RETURN(HSTR_RESULT_DOMAIN_OUT_OF_RANGE);
        }

        *out_pNumLogStreams = log_dom->getNumLogStreams();

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }

}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetLogStreamIDList , 1)(
    HSTR_LOG_DOM  in_LogDomainID,
    uint32_t      in_NumLogStreams,
    HSTR_LOG_STR *out_pLogStreamIDs)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogDomainID);
        HSTR_TRACE_FUN_ARG(in_NumLogStreams);
        HSTR_TRACE_FUN_ARG(out_pLogStreamIDs);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }
        if (out_pLogStreamIDs == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogDomain *log_domain = log_domains.lookupByLogDomainID(in_LogDomainID);

        if (NULL == log_domain) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        uint32_t num_written, num_present;
        log_domain->getLogStreamIDs(out_pLogStreamIDs, in_NumLogStreams, &num_written, &num_present);

        if (num_written < in_NumLogStreams) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        if (num_present > in_NumLogStreams) {
            HSTR_RETURN(HSTR_RESULT_INCONSISTENT_ARGS);
        }
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetLogStreamDetails , 1)(
    HSTR_LOG_STR      in_LogStreamID,
    HSTR_LOG_DOM      /*in_LogDomainID*/,
    HSTR_CPU_MASK     out_CPUmask)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(out_CPUmask);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        if (out_CPUmask    == NULL) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        } // no point in checking out_CPUmask since it's an array, not a pointer

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
        if (NULL == log_stream) {
            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        memcpy(out_CPUmask, log_stream->getCPUMask().mask, sizeof(HSTR_CPU_MASK));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

#define MISC_DATA_ELTS HSTR_MISC_DATA_SIZE/sizeof(uint64_t)

static HSTR_RESULT
hStreams_EnqueueCompute_worker(
    HSTR_LOG_STR        in_LogStreamID,
    const char         *in_pFunctionName,
    uint32_t            in_numScalarArgs,
    uint32_t            in_numHeapArgs,
    uint64_t           *in_pArgs,
    HSTR_EVENT         *out_pEvent,
    void               *out_ReturnValue,
    uint16_t            in_ReturnValueSize,
    int                 hstreams_library_version)
{
    if (hstreams_library_version != HSTR_SECOND_PREVIEW_LIBRARY_VERSION) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "hStreams_EnqueueCompute was called with an unsupported hStreams library version: "
                << hstreams_library_version;

        // unless _hStreams_FatalError is overridden, we should never get here
        return HSTR_RESULT_INTERNAL_ERROR;
    }

    // Error checking
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_pFunctionName == NULL) {
        return HSTR_RESULT_BAD_NAME;
    }
    if (in_ReturnValueSize > HSTR_RETURN_SIZE_LIMIT) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    if (in_numScalarArgs + in_numHeapArgs && !in_pArgs) {
        return HSTR_RESULT_NULL_PTR;
    }
    if (((in_ReturnValueSize > 0)  && (out_ReturnValue == NULL)) ||
            ((in_ReturnValueSize == 0) && (out_ReturnValue != NULL))) {
        return HSTR_RESULT_INCONSISTENT_ARGS;
    }
    if (strlen(in_pFunctionName) > HSTR_MAX_FUNC_NAME_SIZE - 1) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Sorry, " << in_pFunctionName << " exceeds max called function name size of "
                << HSTR_MAX_FUNC_NAME_SIZE - 1;

        return HSTR_RESULT_BAD_NAME;
    }
    uint64_t spt = HSTR_ARGS_SUPPORTED;
    if (spt > HSTR_ARGS_IMPLEMENTED) {
        spt = HSTR_ARGS_IMPLEMENTED;
    }
    uint64_t requestedArgs = (uint64_t)3 + (uint64_t)in_numScalarArgs + (uint64_t)in_numHeapArgs;
    if (requestedArgs > spt) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Sorry, implementation only supports no more than (# scalar + # heap) = "
                << spt << " arguments to streamed functions.";

        return HSTR_RESULT_TOO_MANY_ARGS;
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_stream) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Did not find a logical stream " << in_LogStreamID;

        return HSTR_RESULT_NOT_FOUND;
    }

    hStreams_LogDomain &log_domain = log_stream->getLogDomain();

    std::vector<uint64_t> scalar_args;
    scalar_args.assign(in_pArgs, in_pArgs + in_numScalarArgs);

    std::vector<uint64_t> buffer_offsets;
    buffer_offsets.reserve(in_numHeapArgs);
    std::vector<hStreams_PhysBuffer *> buffer_args;
    buffer_args.reserve(in_numHeapArgs);
    for (uint64_t i = in_numScalarArgs; i < in_numScalarArgs + in_numHeapArgs; ++i) {
        uint64_t addr = in_pArgs[i];

        hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(addr);
        if (!log_buf) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a corresponding buffer for argument #"
                    << i << " : " << (void *) in_pArgs[i];

            return HSTR_RESULT_NOT_FOUND;
        }
        hStreams_PhysBuffer *phys_buf = log_buf->getPhysBufferForLogDomain(log_domain);
        if (!phys_buf) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a corresponding buffer for argument #"
                    << i << " on logical domain # " << log_domain.id();

            return HSTR_RESULT_NOT_FOUND;
        }
        buffer_args.push_back(phys_buf);
        // NOTE Those are offsets into the source buffers.
        //      Physical buffers will compensate for eventual sink-side
        //      buffer padding themselves.
        uint64_t sink_offset = addr - (uint64_t)log_buf->getStart();
        buffer_offsets.push_back(sink_offset);
    }

    hStreams_PhysStream &phys_stream = log_stream->getPhysStream();

    return phys_stream.enqueueFunction(in_pFunctionName, scalar_args, buffer_args,
                                       buffer_offsets, out_ReturnValue, (int16_t) in_ReturnValueSize, out_pEvent);
}

#ifndef _WIN32
HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EnqueueCompute , 1)(
    HSTR_LOG_STR        in_LogStreamID,
    const char         *in_pFunctionName,
    uint32_t            in_numScalarArgs,
    uint32_t            in_numHeapArgs,
    uint64_t           *in_pArgs,
    HSTR_EVENT         *out_pEvent,
    void               *out_ReturnValue,
    uint32_t            in_ReturnValueSize)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG_STR(in_pFunctionName);
    HSTR_TRACE_FUN_ARG(in_numScalarArgs);
    HSTR_TRACE_FUN_ARG(in_numHeapArgs);
    HSTR_TRACE_FUN_ARG(in_pArgs);
    HSTR_TRACE_FUN_ARG(out_pEvent);
    HSTR_TRACE_FUN_ARG(out_ReturnValue);
    HSTR_TRACE_FUN_ARG(in_ReturnValueSize);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

    HSTR_RETURN(hStreams_EnqueueCompute_worker(in_LogStreamID,
                in_pFunctionName,
                in_numScalarArgs,
                in_numHeapArgs,
                in_pArgs,
                out_pEvent,
                out_ReturnValue,
                in_ReturnValueSize,
                HSTR_SECOND_PREVIEW_LIBRARY_VERSION));
}
#endif

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EnqueueCompute , 2)(
    HSTR_LOG_STR        in_LogStreamID,
    const char         *in_pFunctionName,
    uint32_t            in_numScalarArgs,
    uint32_t            in_numHeapArgs,
    uint64_t           *in_pArgs,
    HSTR_EVENT         *out_pEvent,
    void               *out_ReturnValue,
    uint16_t            in_ReturnValueSize)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG_STR(in_pFunctionName);
        HSTR_TRACE_FUN_ARG(in_numScalarArgs);
        HSTR_TRACE_FUN_ARG(in_numHeapArgs);
        HSTR_TRACE_FUN_ARG(in_pArgs);
        HSTR_TRACE_FUN_ARG(out_pEvent);
        HSTR_TRACE_FUN_ARG(out_ReturnValue);
        HSTR_TRACE_FUN_ARG(in_ReturnValueSize);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        HSTR_RETURN(hStreams_EnqueueCompute_worker(in_LogStreamID,
                    in_pFunctionName,
                    in_numScalarArgs,
                    in_numHeapArgs,
                    in_pArgs,
                    out_pEvent,
                    out_ReturnValue,
                    in_ReturnValueSize,
                    HSTR_SECOND_PREVIEW_LIBRARY_VERSION));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

// This just expects the logical stream and the logical domains to have been
// looked up and the appropriate locks to have been grabbed. It is the common
// part for EnqueueData1D and EnqueueDataXDomain1D
static HSTR_RESULT hStreams_EnqueueDataXDomain1D_worker_locked(
    hStreams_LogStream &in_LogStream,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    hStreams_LogDomain &in_dstLogDomain,
    hStreams_LogDomain &in_srcLogDomain,
    HSTR_EVENT         *out_pEvent)
{
    if (in_pWriteAddr == NULL || in_pReadAddr == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    if (0 == in_size) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }

    if (in_LogStream.getLogDomain().id() != in_srcLogDomain.id() &&
            in_LogStream.getLogDomain().id() != in_dstLogDomain.id()) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "The logical stream does not belong in any of the specified domains.";

        return HSTR_RESULT_OUT_OF_RANGE;
    }

    uint64_t src_u64 = (uint64_t)in_pReadAddr;
    uint64_t dst_u64 = (uint64_t)in_pWriteAddr;

    if (in_srcLogDomain.id() == in_dstLogDomain.id()) {
        // Note for future: also for aliased buffers
        // do arithmetic on a pointer to byte-sized type as the transfers are with that granularity
        if (add_gt(src_u64, in_size, dst_u64) || add_gt(dst_u64, in_size, src_u64)) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Overlapping transfers within the same logical domain are not permitted.";

            return HSTR_RESULT_OUT_OF_RANGE;
        }
    }

    // implementation note: though we _could_ use lookupLogBuffer(start, size,
    // overlap), the error codes exposed by the EnqueueData* APIs require that
    // HSTR_RESULT_OUT_OF_RANGE be returned if "in_size extends past the end of
    // an allocated buffer". So we have to do the checking by hand here :/

    hStreams_LogBuffer *dst_log_buf = log_buffers.lookupLogBuffer(in_pWriteAddr);
    if (NULL == dst_log_buf) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Did not find destination buffer.";

        return HSTR_RESULT_NOT_FOUND;
    }
    if (add_gt(dst_u64, in_size, dst_log_buf->getStartu64() + dst_log_buf->getLen())) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Destination range extends past the end of the allocated buffer.";

        return HSTR_RESULT_OUT_OF_RANGE;
    }

    hStreams_PhysBuffer *dst_phys_buf = dst_log_buf->getPhysBufferForLogDomain(in_dstLogDomain);
    if (NULL == dst_phys_buf) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Did not find an instantiation of the destination buffer for logical domain #"
                << in_dstLogDomain.id();

        return HSTR_RESULT_NOT_FOUND;
    }

    hStreams_LogBuffer *src_log_buf = log_buffers.lookupLogBuffer(in_pReadAddr);
    if (NULL == src_log_buf) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Did not find source buffer.";

        return HSTR_RESULT_NOT_FOUND;
    }
    if (add_gt(src_u64, in_size, src_log_buf->getStartu64() + src_log_buf->getLen())) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Destination range extends past the end of the allocated buffer.";

        return HSTR_RESULT_OUT_OF_RANGE;
    }

    hStreams_PhysBuffer *src_phys_buf = src_log_buf->getPhysBufferForLogDomain(in_srcLogDomain);
    if (NULL == src_phys_buf) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Did not find an instantiation of the source buffer for logical domain #"
                << in_srcLogDomain.id();

        return HSTR_RESULT_NOT_FOUND;
    }

    hStreams_PhysStream &phys_stream = in_LogStream.getPhysStream();

    uint64_t dst_offset = (uint64_t)in_pWriteAddr - dst_log_buf->getStartu64();
    uint64_t src_offset = (uint64_t)in_pReadAddr - src_log_buf->getStartu64();

    return phys_stream.enqueueTransfer(*dst_phys_buf, *src_phys_buf, dst_offset,
                                       src_offset, in_size, out_pEvent);
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EnqueueData1D , 1)(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_pReadAddr);
        HSTR_TRACE_FUN_ARG(in_size);
        HSTR_TRACE_FUN_ARG(in_XferDirection);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
        if (NULL == log_stream) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a logical stream #" << in_LogStreamID;

            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }
        hStreams_LogDomain &other_log_domain = log_stream->getLogDomain();

        hStreams_LogDomain *src_log_domain = log_domains.lookupByLogDomainID(HSTR_SRC_LOG_DOMAIN);
        // Let's be paranoid together
        if (NULL == src_log_domain) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "The logical domain of origin for the buffer's transfer doesn't exist";

            HSTR_RETURN(HSTR_RESULT_INTERNAL_ERROR);
        }

        hStreams_LogDomain *xfer_src_log_domain;
        hStreams_LogDomain *xfer_dst_log_domain;

        switch (in_XferDirection) {
        case HSTR_SRC_TO_SINK:
            xfer_src_log_domain = src_log_domain;
            xfer_dst_log_domain = &other_log_domain;
            break;
        case HSTR_SINK_TO_SRC:
            xfer_src_log_domain = &other_log_domain;
            xfer_dst_log_domain = src_log_domain;
            break;
        default:
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Invalid value for in_XferDirection: " << in_XferDirection
                    << ". Valid values are: HSTR_SRC_TO_SINK (" << HSTR_SRC_TO_SINK
                    << ") and HSTR_SINK_TO_SRC (" << HSTR_SINK_TO_SRC << ").";

            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        } // switch (in_XferDirection)

        HSTR_RETURN(hStreams_EnqueueDataXDomain1D_worker_locked(*log_stream,
                    in_pWriteAddr, in_pReadAddr, in_size, *xfer_dst_log_domain, *xfer_src_log_domain, out_pEvent));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EnqueueDataXDomain1D , 1)(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_LOG_DOM        in_dstLogDomain,
    HSTR_LOG_DOM        in_srcLogDomain,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_pReadAddr);
        HSTR_TRACE_FUN_ARG(in_size);
        HSTR_TRACE_FUN_ARG(in_dstLogDomain);
        HSTR_TRACE_FUN_ARG(in_srcLogDomain);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);
        hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
                hStreams_RW_Lock::HSTR_RW_LOCK_READ);

        hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
        if (NULL == log_stream) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a logical stream #" << in_LogStreamID;

            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        hStreams_LogDomain *xfer_dst_log_domain = log_domains.lookupByLogDomainID(in_dstLogDomain);
        if (NULL == xfer_dst_log_domain) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a logical domain " << in_dstLogDomain;

            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }

        hStreams_LogDomain *xfer_src_log_domain = log_domains.lookupByLogDomainID(in_srcLogDomain);
        if (NULL == xfer_src_log_domain) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Did not find a logical domain " << in_srcLogDomain;

            HSTR_RETURN(HSTR_RESULT_NOT_FOUND);
        }
        HSTR_RETURN(hStreams_EnqueueDataXDomain1D_worker_locked(*log_stream,
                    in_pWriteAddr, in_pReadAddr, in_size, *xfer_dst_log_domain, *xfer_src_log_domain, out_pEvent));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT
hStreams_StreamSynchronize_worker(HSTR_LOG_STR in_LogStreamID)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);

    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    // grabbing the streams lock for writing ensures that nobody commits any
    // actions to any streams. This a temporary solution; Alternatively, one could
    // create a lock for each logical stream and acquire that here (or even provide a
    // "synchronize" method) in the logical stream (preferred), but I don't have a clear
    // idea on how to handle thread synchronize, where we basically have to gather all the
    // events from all the streams
    //
    // One can relax that after EventStreamWait functionality is implemented inside the hStreams_PhysStream class, internally synchronized.
    // That would allow more concurrency.
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_stream) {
        return HSTR_RESULT_NOT_FOUND;
    }

    std::vector<HSTR_EVENT> the_events;
    log_stream->getAllEvents(the_events);

    if (the_events.empty()) {
        return HSTR_RESULT_SUCCESS;
    }

    int32_t timeout = (int32_t)hStreams_GetOptions_time_out_ms_val();
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait((int16_t) the_events.size(),
                            &the_events[0], timeout, true, NULL, NULL);

    if (result == HSTR_COI_PROCESS_DIED) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Sink-side process died.";

        return HSTR_RESULT_REMOTE_ERROR;
    } else if (result == HSTR_COI_EVENT_CANCELED) {
        return HSTR_RESULT_EVENT_CANCELED;
    } else if (result == HSTR_COI_SUCCESS) {
        return HSTR_RESULT_SUCCESS;
    } else if (result == HSTR_COI_TIME_OUT_REACHED) {
        HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                << "Time out value of " << timeout
                << " was not large enough, try adjusting time_out_ms_val in HSTR_OPTIONS";

        return HSTR_RESULT_TIME_OUT_REACHED;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_StreamSynchronize , 1)(
    HSTR_LOG_STR      in_LogStreamID)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
        HSTR_RETURN(hStreams_StreamSynchronize_worker(in_LogStreamID));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT hStreams_ThreadSynchronize_worker()
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    // See StreamSynchronize_worker for detailed explanation about the write lock
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    std::vector<HSTR_EVENT> all_the_events;
    log_streams.getEventsFromAllStreams(all_the_events);

    if (all_the_events.empty()) {
        return HSTR_RESULT_SUCCESS;
    }

    int32_t timeout = (int32_t)hStreams_GetOptions_time_out_ms_val();
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait((int16_t) all_the_events.size(),
                            &all_the_events[0], timeout, true, NULL, NULL);

    if (result == HSTR_COI_PROCESS_DIED) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Sink-side process died.";

        return HSTR_RESULT_REMOTE_ERROR;
    } else if (result == HSTR_COI_EVENT_CANCELED) {
        return HSTR_RESULT_EVENT_CANCELED;
    } else if (result == HSTR_COI_SUCCESS) {
        return HSTR_RESULT_SUCCESS;
    } else if (result == HSTR_COI_TIME_OUT_REACHED) {
        HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                << "Time out value of " << timeout
                << " was not large enough, try adjusting time_out_ms_val in HSTR_OPTIONS.";

        return HSTR_RESULT_TIME_OUT_REACHED;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_ThreadSynchronize , 1)()
{
    try {
        HSTR_TRACE_FUN_ENTER();

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
        HSTR_RETURN(hStreams_ThreadSynchronize_worker());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EventWait , 1)(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    bool               in_WaitForAll,
    int32_t            in_TimeOutMilliSeconds,
    uint32_t          *out_pNumSignaled,
    uint32_t          *out_pSignaledIndices)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_NumEvents);
        HSTR_TRACE_FUN_ARG(in_pEvents);
        HSTR_TRACE_FUN_ARG(in_WaitForAll);
        HSTR_TRACE_FUN_ARG(in_TimeOutMilliSeconds);
        HSTR_TRACE_FUN_ARG(out_pNumSignaled);
        HSTR_TRACE_FUN_ARG(out_pSignaledIndices);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
        // Look up the event and make sure it's valid
        HSTR_COIRESULT result;

        if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_NOT_INITIALIZED);
        }

        if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_NONE) { // Ignore dependencies
            HSTR_LOG(HSTR_INFO_TYPE_SYNC) << "Dependencies ignored in EventWait";

            HSTR_RETURN(HSTR_RESULT_SUCCESS);
        }

        // Do a COIEventWait on it
        result = hStreams_COIWrapper::COIEventWait(
                     (uint16_t)in_NumEvents,
                     in_pEvents,
                     in_TimeOutMilliSeconds,
                     in_WaitForAll,
                     out_pNumSignaled,
                     out_pSignaledIndices);

        if (result == HSTR_COI_SUCCESS) {
            HSTR_RETURN(HSTR_RESULT_SUCCESS);
        }

        if (result == HSTR_COI_EVENT_CANCELED) {
            HSTR_RETURN(HSTR_RESULT_EVENT_CANCELED);
        }
        if (result == HSTR_COI_OUT_OF_RANGE) {
            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }
        if (result == HSTR_COI_INVALID_POINTER) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }
        if (result == HSTR_COI_TIME_OUT_REACHED) {
            HSTR_RETURN(HSTR_RESULT_TIME_OUT_REACHED);
        }
        if (result == HSTR_COI_ARGUMENT_MISMATCH) {
            HSTR_RETURN(HSTR_RESULT_INCONSISTENT_ARGS);
        }
        if (result == HSTR_COI_PROCESS_DIED) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Sink-side process died.";

            HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
        }

        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Internal error, COI returned an unexpected value: "
                << hStreams_COIWrapper::COIResultGetName(result);

        // unless _hStreams_FatalError is overridden, we should never get here
        HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT
hStreams_EventStreamWait_worker(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT        *out_pEvent)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (((in_NumEvents == 0) != (in_pEvents == NULL)) || // can't be NULL if > 0
            ((in_NumAddresses <= 0) != (in_pAddresses == NULL)) || // can't be NULL if > 0
            ((in_NumAddresses == HSTR_WAIT_NONE) && (out_pEvent == NULL))) { // can't be NULL if no deps
        return HSTR_RESULT_INCONSISTENT_ARGS;
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_stream) {
        return HSTR_RESULT_NOT_FOUND;
    }

    hStreams_PhysStream &phys_stream = log_stream->getPhysStream();
    hStreams_LogDomain &log_domain = log_stream->getLogDomain();

    std::vector<hStreams_PhysBuffer *> phys_buffers;
    if (in_NumAddresses > 0) {
        phys_buffers.reserve(in_NumAddresses);
        for (int32_t i = 0; i < in_NumAddresses; ++i) {
            hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_pAddresses[i]);
            if (NULL == log_buf) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                        << "Did not find buffer for argument: " << i
                        << " ,address: " << in_pAddresses[i];

                return HSTR_RESULT_NOT_FOUND;
            }
            hStreams_PhysBuffer *phys_buf = log_buf->getPhysBufferForLogDomain(log_domain);
            if (NULL == phys_buf) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                        << "Did not find instantiation of buffer for argument " << i
                        << "for logical domain #" << log_domain.id();

                return HSTR_RESULT_NOT_FOUND;
            }
            phys_buffers.push_back(phys_buf);
        }
    }

    if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_NONE) { // Ignore dependencies
        HSTR_LOG(HSTR_INFO_TYPE_SYNC) << "Dependencies ignored in EventStreamWait";

        return HSTR_RESULT_SUCCESS;
    }

    // If no events or a control dep follows, make depend on all preceding
    // If deps on all previous were to not be enforced in the
    //  in_NumAddresses == HSTR_WAIT_CONTROL case,  previously-specified
    //  dependences would be lost: we'd be depending only on the wait's predecessors
    DEP_TYPE dep_type = (in_NumEvents == HSTR_WAIT_CONTROL ||
                         in_NumAddresses == HSTR_WAIT_CONTROL)
                        ? IS_BARRIER : IS_XFER;

    std::vector<HSTR_EVENT> events;

    phys_stream.getInputDeps(dep_type, phys_buffers, events);

    if (in_NumEvents > 0) {
        events.insert(events.end(), in_pEvents, in_pEvents + in_NumEvents);
    }

    HSTR_EVENT completion;
    // Create an action, COIBufferWrite, that manages the dependences Perform a
    // COIBufferWrite is cheaper than COIPipelineRunFunction, and has the
    // desired semantics COIBufferRead would be ok too.  Make sure it's the
    // best-performing direction - from host?  Make the Buffer referenced here
    // be a global dummy, of size 1 (not allowed to be 0) Let the input
    // dependences be the Events, plus the valid pending COIEvents for each of
    // the intersecting buffers Let the output dependence of that action be the
    // completion event, allocated above
    // XXX Does using the same dummy_buf for all streams across all domains not
    // introduce some weird dependency between unrelated streams?
    HSTR_COIRESULT result = hStreams_COIWrapper::COIBufferWrite(
                                hstr_proc.dummy_buf,                    // in_DestBuffer
                                0,                                      // in_Offset
                                &hstr_proc.dummy_data,                  // in_pSourceData. Cannot be NULL
                                1,                                      // in_Length
                                HSTR_COI_COPY_USE_CPU,                       // in_Type
                                (uint32_t) events.size(),               // total number of dependencies
                                (events.empty()) ? NULL : &events[0],   // in_pDependencies
                                &completion);                          // out_pCompletion

    // Result checking
    if (result != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "A problem encountered while writing data to buffer: "
                << hStreams_COIWrapper::COIResultGetName(result);

        HSTR_RETURN(HSTR_RESULT_REMOTE_ERROR);
    }

    // Iterate through addresses; there may be none
    // Turn each address into a buffer index, else return HSTR_RESULT_NOT_FOUND
    // The result, bufferArray is an array of intersecting buffer indices

    // Update deps for that stream
    // Call setOutputDeps
    if (in_NumAddresses == HSTR_WAIT_CONTROL) {
        // Need to pass all the physical buffers instantiated for this logical domain
        // to setOutputDeps; re-use the phys_buffers vector
        log_buffers.getAllPhysBuffersForLogDomain(log_domain, phys_buffers);
        dep_type = IS_BARRIER;
    } else if (in_NumAddresses == HSTR_WAIT_NONE) {
        dep_type = NONE;
    } else {
        dep_type = IS_XFER;
    }

    // Pass in the list of intersecting buffers, which is non-NULL if in_NumAddresses > 0
    // Pass in the completion event from the dummy COIBufferWrite action
    if (dep_type != NONE) {
        phys_stream.setOutputDeps(dep_type, phys_buffers, completion);
    }

    //
    // If caller wants to wait for completion, pass back the handle.
    //
    if (out_pEvent != NULL) {
        *out_pEvent = completion;
    }

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_EventStreamWait , 1)(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT        *out_pEvent)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_NumEvents);
        HSTR_TRACE_FUN_ARG(in_pEvents);
        HSTR_TRACE_FUN_ARG(in_NumAddresses);
        HSTR_TRACE_FUN_ARG(in_pAddresses);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        HSTR_RETURN(hStreams_EventStreamWait_worker(
                        in_LogStreamID,
                        in_NumEvents,
                        in_pEvents,
                        in_NumAddresses,
                        in_pAddresses,
                        out_pEvent));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT hStreams_Alloc1DEx_worker(
    void                    *in_BaseAddress,
    uint64_t                 in_Size,
    const HSTR_BUFFER_PROPS &in_pBufferProps,
    int64_t                  in_NumLogDomains,
    HSTR_LOG_DOM            *in_pLogDomainIDs)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_Size == 0 || in_NumLogDomains < -1) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    if (addition_overflow((uint64_t)in_BaseAddress, in_Size)) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    if (in_BaseAddress == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    if (((in_NumLogDomains == -1 || in_NumLogDomains == 0) && (in_pLogDomainIDs != NULL))
            || (!(in_NumLogDomains == -1 || in_NumLogDomains == 0) && (in_pLogDomainIDs == NULL))) {
        return HSTR_RESULT_INCONSISTENT_ARGS;
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }
    }
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    std::tr1::unordered_set<hStreams_LogDomain *> log_domains_set;

    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            return HSTR_RESULT_INCONSISTENT_ARGS;
        }
    }

    HSTR_OVERLAP_TYPE buf_overlap = HSTR_NO_OVERLAP;
    log_buffers.lookupLogBuffer(in_BaseAddress, in_Size, &buf_overlap);
    if (HSTR_EXACT_OVERLAP == buf_overlap) {
        return HSTR_RESULT_ALREADY_FOUND;
    }
    if (HSTR_PARTIAL_OVERLAP == buf_overlap) {
        return HSTR_RESULT_OVERLAPPING_RESOURCES;
    }

    hStreams_LogBuffer *log_buf = new hStreams_LogBuffer(in_BaseAddress, in_Size, in_pBufferProps);

    // Attach all selected logical domains
    if (in_NumLogDomains == -1) {
        CHECK_HSTR_RESULT(
            log_buf->attachLogDomainFromRange(log_domains.begin(), log_domains.end()));
    } else {
        // Buffer will always be created for HSTR_SRC_LOG_DOMAIN, and it can't be in in_pLogDomainIDs
        // so it is added manually here
        hStreams_LogDomain *src_log_dom = log_domains.lookupByLogDomainID(HSTR_SRC_LOG_DOMAIN);
        log_domains_set.insert(src_log_dom);
        CHECK_HSTR_RESULT(
            log_buf->attachLogDomainFromRange(log_domains_set.begin(), log_domains_set.end()));
    }

    log_buffers.addToCollection(log_buf);

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Alloc1D , 1)(
    void               *in_BaseAddress,
    uint64_t            in_size)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_BaseAddress);
        HSTR_TRACE_FUN_ARG(in_size);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        // Use default Alloc1D properties and create buffer for all logical domains
        HSTR_BUFFER_PROPS buffer_props = HSTR_BUFFER_PROPS_INITIAL_VALUES;

        HSTR_RETURN(hStreams_Alloc1DEx_worker(
                        in_BaseAddress,
                        in_size,
                        buffer_props,
                        -1,
                        NULL));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Alloc1DEx, 1)(
    void                 *in_BaseAddress,
    uint64_t              in_Size,
    HSTR_BUFFER_PROPS    *in_pBufferProps,
    int64_t               in_NumLogDomains,
    HSTR_LOG_DOM         *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_BaseAddress);
        HSTR_TRACE_FUN_ARG(in_Size);
        HSTR_TRACE_FUN_ARG(in_pBufferProps);
        HSTR_TRACE_FUN_ARG(in_NumLogDomains);
        HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        // Use default Alloc1DEx properties object if in_pBufferProps == NULL
        if (in_pBufferProps == NULL) {
            HSTR_BUFFER_PROPS default_props = HSTR_BUFFER_PROPS_INITIAL_VALUES_EX;
            in_pBufferProps = &default_props;
        } else {
            if (in_pBufferProps->mem_type != HSTR_MEM_TYPE_NORMAL ||
                    (in_pBufferProps->flags & HSTR_BUF_PROP_AFFINITIZED) != 0) {
                return HSTR_RESULT_NOT_IMPLEMENTED;
            }
        }

        HSTR_RETURN(hStreams_Alloc1DEx_worker(
                        in_BaseAddress,
                        in_Size,
                        *in_pBufferProps,
                        in_NumLogDomains,
                        in_pLogDomainIDs));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

static HSTR_RESULT hStreams_AddBufferLogDomains_worker(
    void            *in_Address,
    uint64_t         in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_NumLogDomains == 0) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    if ((in_Address == NULL) || (in_pLogDomainIDs == NULL)) {
        return HSTR_RESULT_NULL_PTR;
    }
    for (uint64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    std::tr1::unordered_set<hStreams_LogDomain *> log_domains_set;
    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);

    if (log_buf == NULL) {
        return HSTR_RESULT_NOT_FOUND;
    }
    for (uint64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            return HSTR_RESULT_INCONSISTENT_ARGS;
        }

        if (log_buf->getPhysBufferForLogDomain(*log_dom) != NULL) {
            return HSTR_RESULT_ALREADY_FOUND;
        }
    }

    // Attach buffer for selected logical domains
    CHECK_HSTR_RESULT(
        log_buf->attachLogDomainFromRange(log_domains_set.begin(), log_domains_set.end()));

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_AddBufferLogDomains , 1)(
    void            *in_Address,
    uint64_t         in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
    HSTR_RETURN(hStreams_AddBufferLogDomains_worker(
                    in_Address,
                    in_NumLogDomains,
                    in_pLogDomainIDs));
}

static HSTR_RESULT hStreams_RmBufferLogDomains_worker(
    void               *in_Address,
    int64_t             in_NumLogDomains,
    HSTR_LOG_DOM       *in_pLogDomainIDs)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_Address == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    if (in_NumLogDomains == 0 || in_NumLogDomains < -1) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    if (((in_NumLogDomains == - 1) && (in_pLogDomainIDs != NULL))
            || ((in_NumLogDomains != - 1) && (in_pLogDomainIDs == NULL))) {
        return HSTR_RESULT_INCONSISTENT_ARGS;
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    std::tr1::unordered_set<hStreams_LogDomain *> log_domains_set;
    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);

    if (log_buf == NULL) {
        return HSTR_RESULT_NOT_FOUND;
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            return HSTR_RESULT_DOMAIN_OUT_OF_RANGE;
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            return HSTR_RESULT_INCONSISTENT_ARGS;
        }

        if (log_buf->getPhysBufferForLogDomain(*log_dom) == NULL) {
            return HSTR_RESULT_NOT_FOUND;
        }
    }

    // Detach buffer from selected logical domains
    if (in_NumLogDomains == -1) {
        log_buf->detachAllLogDomain();
    } else {
        log_buf->detachLogDomainFromRange(log_domains_set.begin(), log_domains_set.end());
    }

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_RmBufferLogDomains , 1)(
    void               *in_Address,
    int64_t             in_NumLogDomains,
    HSTR_LOG_DOM       *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
    HSTR_RETURN(hStreams_RmBufferLogDomains_worker(
                    in_Address,
                    in_NumLogDomains,
                    in_pLogDomainIDs));
}

static HSTR_RESULT hStreams_GetBufferNumLogDomains_worker(
    void                *in_Address,
    uint64_t            *out_NumLogDomains)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_Address == NULL || out_NumLogDomains == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);
    if (log_buf == NULL) {
        return HSTR_RESULT_NOT_FOUND;
    }

    *out_NumLogDomains = log_buf->getNumAttachedLogDomains();

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetBufferNumLogDomains , 1)(
    void                *in_Address,
    uint64_t            *out_NumLogDomains)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(out_NumLogDomains);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
    HSTR_RETURN(hStreams_GetBufferNumLogDomains_worker(
                    in_Address,
                    out_NumLogDomains));
}

static HSTR_RESULT hStreams_GetBufferLogDomains_worker(
    void                *in_Address,
    uint64_t             in_NumLogDomains,
    HSTR_LOG_DOM        *out_pLogDomains,
    uint64_t            *out_pNumLogDomains)
{
    hStreams_LogBuffer *log_buf = NULL;

    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_Address == NULL || out_pNumLogDomains == NULL || out_pLogDomains == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    if (in_NumLogDomains == 0) {
        return HSTR_RESULT_OUT_OF_RANGE;
    }
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    log_buf = log_buffers.lookupLogBuffer(in_Address);
    if (log_buf == NULL) {
        return HSTR_RESULT_NOT_FOUND;
    }

    log_buf->getAttachedLogDomainIDs(in_NumLogDomains, out_pLogDomains, out_pNumLogDomains);

    if (in_NumLogDomains < *out_pNumLogDomains) {
        return HSTR_RESULT_INCONSISTENT_ARGS;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetBufferLogDomains , 1)(
    void                *in_Address,
    uint64_t             in_NumLogDomains,
    HSTR_LOG_DOM        *out_pLogDomains,
    uint64_t            *out_pNumLogDomains)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(out_pLogDomains);
    HSTR_TRACE_FUN_ARG(out_pNumLogDomains);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
    HSTR_RETURN(hStreams_GetBufferLogDomains_worker(
                    in_Address,
                    in_NumLogDomains,
                    out_pLogDomains,
                    out_pNumLogDomains));
}

static HSTR_RESULT  hStreams_GetBufferProps_worker(
    void                *in_Address,
    HSTR_BUFFER_PROPS   *out_BufferProps)
{
    hStreams_LogBuffer *log_buf = NULL;

    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (in_Address == NULL || out_BufferProps == NULL) {
        return HSTR_RESULT_NULL_PTR;
    }
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    log_buf = log_buffers.lookupLogBuffer(in_Address);
    if (log_buf == NULL) {
        return HSTR_RESULT_NOT_FOUND;
    }

    *out_BufferProps = log_buf->getProperties();

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetBufferProps , 1)(
    void                *in_Address,
    HSTR_BUFFER_PROPS   *out_BufferProps)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(out_BufferProps);

    hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
    HSTR_RETURN(hStreams_GetBufferProps_worker(
                    in_Address,
                    out_BufferProps));
}

static HSTR_RESULT hStreams_DeAlloc_worker(void *in_Address)
{
    if (hStreams_IsInitialized() != HSTR_RESULT_SUCCESS) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
    if (!in_Address) {
        return HSTR_RESULT_NULL_PTR;
    }
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);
    if (!log_buf) {
        return HSTR_RESULT_NOT_FOUND;
    }
    log_buffers.delFromCollection(*log_buf);
    delete log_buf;
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_DeAlloc , 1)(
    void               *in_Address)
{
    try {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_Address);

        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);
        HSTR_RETURN(hStreams_DeAlloc_worker(in_Address));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_GetLastError , 1)()
{
    HSTR_TRACE_FUN_ENTER();

    return (HSTR_RESULT) hstr_proc.lastError;
}

void
HSTR_SYMBOL_VERSION(hStreams_ClearLastError , 1)()
{
    HSTR_TRACE_FUN_ENTER();

    hStreams_AtomicStore(hstr_proc.lastError, HSTR_RESULT_SUCCESS);
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Cfg_SetLogLevel, 1)(
    HSTR_LOG_LEVEL   in_loglevel)
{
    try {
        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_loglevel);

        if (hStreams_IsInitialized() == HSTR_RESULT_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "hStreams_Cfg_SetLogLevel() cannot "
                    << "be called if the library has been already initialized.";

            HSTR_RETURN(HSTR_RESULT_NOT_PERMITTED);
        }
        if (in_loglevel < HSTR_LOG_LEVEL_NO_LOGGING
                || in_loglevel > HSTR_LOG_LEVEL_DEBUG4) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE) << "Invalid value for the logging level";

            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }
        globals::logging_level = in_loglevel;
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Cfg_SetLogInfoType, 1)(
    uint64_t        in_info_type_mask)
{
    try {
        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_info_type_mask);

        if (hStreams_IsInitialized() == HSTR_RESULT_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "hStreams_Cfg_SetLogInfoType() cannot "
                    << "be called if the library has been already initialized.";

            HSTR_RETURN(HSTR_RESULT_NOT_PERMITTED);
        }
        globals::logging_bitmask = in_info_type_mask;
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_RESULT
HSTR_SYMBOL_VERSION(hStreams_Cfg_SetMKLInterface, 1)(
    HSTR_MKL_INTERFACE in_MKLInterface)
{
    try {
        hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter);

        if ((in_MKLInterface < 0) || (HSTR_MKL_INTERFACE_SIZE <= in_MKLInterface)) {

            HSTR_ERROR(HSTR_INFO_TYPE_TRACE) << "Invalid value for the MKL interface";
            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }
        if (hStreams_IsInitialized() == HSTR_RESULT_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "hStreams_Cfg_SetMKLInterface() cannot "
                    << "be called if the library has been already initialized.";
            HSTR_RETURN(HSTR_RESULT_NOT_PERMITTED);
        }
        globals::mkl_interface = in_MKLInterface;
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

#ifdef __cplusplus
}
#endif
