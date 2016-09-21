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
#ifndef _WIN32
#include <dlfcn.h>
#include <unistd.h>
#endif
#include <unordered_set>

#include "hStreams_core_api_workers_source.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_common.h"
#include "hStreams_Logger.h"
#include "hStreams_PhysDomainHost.h"
#include "hStreams_PhysDomainCOI.h"
#include "hStreams_LogBuffer.h"
#include "hStreams_COIWrapper_types.h"

namespace
{

/**
 * Just a small helper class to guard the state of the hStreamsState atomic
 * Upon construction, sets the value of the atomic to \c HSTR_STATE_INITIALIZING
 * Upon destruction, it sets the value of the atomic to \c HSTR_STATE_UNINITIALIZED,
 * unless its member function \c initSuccessfull() is called in which case the atomic
 * will be set to \c HSTR_STATE_INITIALIZED.
 */
class hStreamsStateGuard
{
    bool is_init_successfull_;

public:
    hStreamsStateGuard()
        : is_init_successfull_(false)
    {
        globals::hStreamsState.SetValue(HSTR_STATE_INITIALIZING);
    }

    void initSuccessfull()
    {
        is_init_successfull_ = true;
    }

    ~hStreamsStateGuard()
    {
        if (is_init_successfull_) {
            globals::hStreamsState.SetValue(HSTR_STATE_INITIALIZED);
        } else {
            globals::hStreamsState.SetValue(HSTR_STATE_UNINITIALIZED);
        }
    }

};

} // anonymous namespace

void
detail::InitInVersion_impl_throw(const char *interface_version)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG_STR(interface_version);
    // NOTE: the following declaration introduces a critical section of code.  While the local
    // variable: 'initLock' is in scope, the code will serialize all threads of execution.  The net
    // effect is hStreams_InitInVersion_impl will always operate in a single thread of execution.
    hStreams_Scope_Locker_Unlocker initLock(hstr_proc.hStreamsInitLock);

    // short-circuit if already initialized
    if (hStreams_IsInitialized() == HSTR_RESULT_SUCCESS) {
        HSTR_DEBUG4(HSTR_INFO_TYPE_TRACE) << "Redundant call to hStreams_InitInVersion()";
        return;
    }

    hStreamsStateGuard state_guard;

    if (interface_version == NULL) {
        HSTR_DEBUG2(HSTR_INFO_TYPE_MISC)
                << "Interface version argument in hStreams_InitInVersion()"
                << " is NULL, will use the highest interface version available, "
                << "i.e. the version of the library: "
                << library_version_string;
        interface_version = library_version_string;
    }

    std::string interface_version_str = interface_version;

    if (!detail::in_container(globals::supported_interface_versions, interface_version_str)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Interface version "
                                   << interface_version_str
                                   << " is not a supported."
                                  );
    }

    globals::interface_version = interface_version_str;
    HSTR_LOG(HSTR_INFO_TYPE_MISC) << "Interface version has been set to " << interface_version_str;

    // Constructor will load COI library if it hasn't been loaded already
    new hStreams_COIWrapper;

    // Clear any error if previously set.
    hStreams_ClearLastError();


    assert(0 == (((uint64_t)&hstr_proc.dummy_buf) & 63));
    assert(0 == (((uint64_t)&hstr_proc.dummy_data) & 63));

    uint32_t            active_domains_knc, num_phys_domains_knc,
                        active_domains_x200, num_phys_domains_x200;
    string              executableFileName;
    HSTR_RESULT         hsr;
    HSTR_COIRESULT      result;
    HSTR_COIPROCESS     dummy_process_knc = NULL, dummy_process_x200 = NULL, dummy_process = NULL;

    if ((hsr = hStreams_FetchExecutableName(executableFileName)) != HSTR_RESULT_SUCCESS) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Cannot obtain executable name.";

        executableFileName.assign("");
    } else {
        HSTR_LOG(HSTR_INFO_TYPE_MISC) << "Executable file name: '" << executableFileName << "'.";
    }
    hStreams_ClearLastError();

    // Set search paths for libraries from environment variables
    setSearchedPaths();

    InitPhysicalDomains_impl_throw(HSTR_ISA_KNC, executableFileName,
                                   "x100_card_startup", (void *)x100_card_startup, x100_card_startup_size,
                                   active_domains_knc, dummy_process_knc, num_phys_domains_knc);

    InitPhysicalDomains_impl_throw(HSTR_ISA_KNL, executableFileName,
                                   "x200_card_startup", (void *)x200_card_startup, x200_card_startup_size,
                                   active_domains_x200, dummy_process_x200, num_phys_domains_x200);

    //This shouldn't happened, cannot have both card types in system.
    if (num_phys_domains_knc * num_phys_domains_x200 > 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DEVICE_NOT_INITIALIZED, StringBuilder()
                                   << "A problem encountered while initalizing physical devices: "
                                   << "System contains both KNC and KNL card. "
                                  );
    }

    hstr_proc.myNumPhysDomains = num_phys_domains_knc ? num_phys_domains_knc : num_phys_domains_x200;
    hstr_proc.myActivePhysDomains = num_phys_domains_knc ? active_domains_knc : active_domains_x200;

    dummy_process = num_phys_domains_knc ? dummy_process_knc : dummy_process_x200;

    std::vector<LIB_HANDLER::handle_t> loaded_libs_handles;
    hStreams_LoadSinkSideLibrariesHost(executableFileName, loaded_libs_handles);

    hStreams_PhysDomain *host_phys_dom = new hStreams_PhysDomainHost(dummy_process, loaded_libs_handles);
    phys_domains.addToCollection(host_phys_dom);
    HSTR_CPU_MASK dummy_mask;
    HSTR_CPU_MASK_ZERO(dummy_mask);
    hStreams_LogDomain *source_log_dom = new hStreams_LogDomain(HSTR_SRC_LOG_DOMAIN, dummy_mask, *host_phys_dom);
    log_domains.addToCollection(source_log_dom);


    if (active_domains_knc == 0 && active_domains_x200 == 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DEVICE_NOT_INITIALIZED, StringBuilder()
                                   << "No active MIC cards in the system. Use of hStreams is not permitted"
                                  );
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
    if (result != HSTR_COI_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DEVICE_NOT_INITIALIZED, StringBuilder()
                                   << "A problem encountered while creating a helper buffer: "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
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

    state_guard.initSuccessfull();
} // hStreams_InitInVersion_impl_throw

void detail::InitPhysicalDomains_impl_throw(HSTR_ISA_TYPE isa_type, std::string executableFileName,
        std::string library_name, void *sink_startup_ptr, uint64_t sink_startup_size,
        uint32_t &active_domains, HSTR_COIPROCESS &dummy_process, uint32_t &num_phys_domains)
{

    uint32_t            i, first_valid_domainID = (uint32_t) - 1;
    const char         *thunk_name                  = "hStreamsThunk";
    const char         *fetchSinkFuncAddress_name   = "hStreams_fetchSinkFuncAddress";
    HSTR_COIRESULT      result;

    // May return HSTR_COI_DOES_NOT_EXIST if isa_type is not matched
    result = hStreams_COIWrapper::COIEngineGetCount(isa_type, &num_phys_domains);

    // For COI from 4.x version COI
    // is returning HSTR_COI_DOES_NOT_EXIST for KNC isa type
    if (result == HSTR_COI_DOES_NOT_EXIST)
    {
        HSTR_DEBUG1(HSTR_INFO_TYPE_MISC) << "Received COI_DOES_NOT_EXIST for given "
            << "isa type: " <<  isa_type;

        num_phys_domains = 0;

        return;
    }

    // Result checking
    if (result != HSTR_COI_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                   << "Cannot initialize. COIEngineGetCount returned "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
    }

    // Artificially constrain the number of domains
    // FIXME: Currently it uses the lowest-indexed domains
    if (num_phys_domains > hStreams_GetOptions_phys_domains_limit()) {
        num_phys_domains = hStreams_GetOptions_phys_domains_limit();
    }

    for (i = 0, active_domains = 0; i < num_phys_domains; i++) {
        HSTR_COIRESULT coi_res;
        HSTR_COIENGINE coi_eng;

        coi_res = hStreams_COIWrapper::COIEngineGetHandle(isa_type, i, &coi_eng);
        if (HSTR_COI_SUCCESS != coi_res) {
            HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
                    << "Skipping physical domain no. "
                    << i
                    << " as COIEngineGetHandle returned "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);
            continue;
        }

        HSTR_COI_ENGINE_INFO coi_eng_info;
        coi_res = hStreams_COIWrapper::COIEngineGetInfo(coi_eng, sizeof(HSTR_COI_ENGINE_INFO), &coi_eng_info);
        if (HSTR_COI_SUCCESS != coi_res) {
            HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
                    << "Skipping physical domain no. "
                    << i
                    << " as COIEngineGetInfo returned "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);
            continue;
        }

        HSTR_COIPROCESS coi_process;
        coi_res = hStreams_COIWrapper::COIProcessCreateFromMemory(coi_eng,
                  library_name.c_str(), sink_startup_ptr, sink_startup_size,
                  0, NULL, false, NULL, true, NULL, 1024 * 1024, globals::target_library_search_path.c_str(),
                  NULL, 0, &coi_process);
        if (HSTR_COI_SUCCESS != coi_res) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                       << "Could not create process on the device; "
                                       << "COIProcessCreateFromMemory returned "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
        }
        std::vector<HSTR_COILIBRARY> loaded_libs;
        HSTR_RESULT hstr_res = hStreams_LoadSinkSideLibrariesMIC(coi_process, loaded_libs,
                               executableFileName, isa_type);
        if (hstr_res != HSTR_RESULT_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(hstr_res, StringBuilder()
                                       << "An error occured while loading libraries on the MIC."
                                      );
        }

        // Get the handle for the thunk, thunk_func
        HSTR_COIFUNCTION thunk_func;
        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &thunk_name, &thunk_func);
        if (coi_res != HSTR_COI_SUCCESS) {
            // FIXME: It seems at this point that we need to do some finalization of the
            // initialization that has happened to this point?
            //
            // We're pushing for RAII in the implementation of the library, we should make
            // auto-destoying COIPROCESS classes and so on and so forth.
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BAD_NAME, StringBuilder()
                                       << "Sink-side library does not contain a function named "
                                       << thunk_name
                                       << ", COIProcessGetFunctionHandles returned "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
        }

        // Get the handle for the fetch sink func address function
        HSTR_COIFUNCTION fetch_addr_func;
        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &fetchSinkFuncAddress_name,
                  &fetch_addr_func);
        if (coi_res != HSTR_COI_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BAD_NAME, StringBuilder()
                                       << "Sink-side library does not contain a function named "
                                       << fetchSinkFuncAddress_name
                                       << ", COIProcessGetFunctionHandles returned "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
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
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                       << "A problem encountered while creating a helper pipeline "
                                       << "on the MIC, COIPipelineCreate returned "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
        }

        HSTR_COIFUNCTION init_sink;
        const char *func_name_init_sink =     "hStreams_init_sink";

        coi_res = hStreams_COIWrapper::COIProcessGetFunctionHandles(coi_process, 1, &func_name_init_sink,
                  &init_sink);
        if (HSTR_COI_SUCCESS != coi_res) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BAD_NAME, StringBuilder()
                                       << "Sink-side library does not contain a function named "
                                       << func_name_init_sink
                                       << ", COIProcessGetFunctionHandles returned "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
        }

        hStreams_InitSinkData init_data;

        init_data.phys_domain_id = active_domains;
        init_data.logging_bitmask = globals::logging_bitmask;
        init_data.logging_level = globals::logging_level;
        init_data.logging_myphysdom = globals::logging_myphysdom;
        init_data.mkl_interface = globals::mkl_interface;

        uint64_t error_code_buf = HSTR_RESULT_SUCCESS;

        coi_res = hStreams_COIWrapper::COIPipelineRunFunction(helper_coi_pipeline, init_sink, 0, NULL, NULL, 0,
                  NULL, &init_data, sizeof(init_data),
                  (void *)&error_code_buf, sizeof(error_code_buf), NULL);
        if (coi_res != HSTR_COI_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                       << "A problem encountered while running a function in the pipeline: "
                                       << hStreams_COIWrapper::COIResultGetName(coi_res)
                                      );
        }

        HSTR_RESULT error_code = (HSTR_RESULT)error_code_buf;

        if (error_code != HSTR_RESULT_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(error_code, StringBuilder()
                                       << "Could not initialize physical domain " << active_domains
                                       << ". Remote process indicated the error code to be: "
                                       << hStreams_ResultGetName(error_code)
                                      );
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

} // hStreams_InitPhysicalDomains_impl_throw

void
detail::IsInitialized_impl_throw()
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_RESULT res = IsInitialized_impl_nothrow();
    if (res != HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_INITIALIZED);
    }
} // hStreams_IsInitialized_impl_throw

HSTR_RESULT
detail::IsInitialized_impl_nothrow()
{
    try {
        HSTR_TRACE_FUN_ENTER();
        if (globals::hStreamsState.GetValue() == HSTR_STATE_INITIALIZED) {
            return HSTR_RESULT_SUCCESS;
        }
        return HSTR_RESULT_NOT_INITIALIZED;
    } catch (...) {
        return HSTR_RESULT_NOT_INITIALIZED;
    }
} // hStreams_IsInitialized_impl_nothrow



void detail::Fini_impl_throw()
{
    HSTR_TRACE_FUN_ENTER();

    // NOTE: the following declaration introduces a critical section of code.
    // While the local variable: 'finiLock' is in scope, the code will serialize
    // all threads of execution.  The net effect is hStreams_Fini will always
    // operate in a single thread of execution.
    hStreams_Scope_Locker_Unlocker finiLock(hstr_proc.hStreamsFiniLock);

    detail::IsInitialized_impl_throw();

    globals::hStreamsState = HSTR_STATE_FINALIZING;

    // Wait until all threads of execution that are executing in a thread of execution
    // in any call of the hStreams library:
    while (hstr_proc.callCounter.GetValue() > 0) {
        hStreams_SleepMS(1);
    }

    HSTR_COIRESULT result = hStreams_COIWrapper::COIBufferDestroy(hstr_proc.dummy_buf);
    if (result != HSTR_COI_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "A problem encountered while destroying the helper buffer, "
                                   << "COIBufferDestroy() returned "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
    }

    log_buffers.destroyAllBuffers();
    log_streams.destroyAllStreams();
    log_domains.destroyAllDomains();
    phys_domains.destroyAllDomains();

    hstr_proc.myActivePhysDomains = 0;
    globals::target_library_search_path.clear();
    globals::host_library_search_path.clear();
    globals::tokenized_target_library_search_path.clear();
    globals::tokenized_host_library_search_path.clear();
    globals::app_init_next_log_str_ID       = globals::initial_values::app_init_next_log_str_ID;
    globals::interface_version              = globals::initial_values::interface_version;
    globals::mkl_interface                  = globals::initial_values::mkl_interface;
    globals::next_log_dom_id                = globals::initial_values::next_log_dom_id;
    globals::options                        = globals::initial_values::options;
    globals::libraries_to_load.clear();
    globals::app_init_log_doms_IDs.clear();

    globals::hStreamsState = HSTR_STATE_UNINITIALIZED;
} // void detail::Fini_impl_throw

void
detail::GetNumPhysDomains_impl_throw(
    uint32_t          *out_pNumPhysDomains,
    uint32_t          *out_pNumActivePhysDomains,
    bool              *out_pHomogeneous)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(out_pNumPhysDomains);
    HSTR_TRACE_FUN_ARG(out_pNumActivePhysDomains);
    HSTR_TRACE_FUN_ARG(out_pHomogeneous);

    detail::IsInitialized_impl_throw();
    if (out_pNumPhysDomains           == NULL ||
            out_pNumActivePhysDomains == NULL ||
            out_pHomogeneous          == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "An output pointer argument of hStreams_GetNumPhysDomains was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    *out_pNumPhysDomains       = hstr_proc.myNumPhysDomains;
    *out_pNumActivePhysDomains = hstr_proc.myActivePhysDomains;
    *out_pHomogeneous =          hstr_proc.homogeneous;
} // detail::GetNumPhysDomains_impl_throw


void
detail::GetPhysDomainDetails_impl_throw(
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
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(out_pNumThreads);
    HSTR_TRACE_FUN_ARG(out_pISA);
    HSTR_TRACE_FUN_ARG(out_pCoreMaxMHz);
    HSTR_TRACE_FUN_ARG(out_MaxCPUmask);
    HSTR_TRACE_FUN_ARG(out_AvoidCPUmask);
    HSTR_TRACE_FUN_ARG(out_pSupportedMemTypes);
    HSTR_TRACE_FUN_ARG(out_pPhysicalBytesPerMemType);

    detail::IsInitialized_impl_throw();

    if (out_pNumThreads                    == NULL ||
            out_pISA                       == NULL ||
            out_pCoreMaxMHz                == NULL ||
            out_pSupportedMemTypes         == NULL ||
            out_pPhysicalBytesPerMemType   == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "An output pointer argument of hStreams_GetPhysDomainDetails was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") not found"
                                  );
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
} // detail::GetPhysDomainDetails_impl_throw(

void
detail::GetOversubscriptionLevel_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumThreads,
    uint32_t       *out_pOversubscriptionArray)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(in_NumThreads);
    HSTR_TRACE_FUN_ARG(out_pOversubscriptionArray);
    IsInitialized_impl_throw();

    if (out_pOversubscriptionArray == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pOversubscriptionArray was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == phys_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") not found"
                                  );
    }

    if (phys_dom->getNumThreads() != in_NumThreads) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "The value of in_NumThreads doesn't match "
                                   << phys_dom->getNumThreads()
                                   << " which is how threads there are in physical domain (ID="
                                   << in_PhysDomainID
                                   << ")"
                                  );
    }
    phys_dom->getOversubscriptionLevel(out_pOversubscriptionArray);
} // detail::GetOversubscriptionLevel_impl_throw(

void
detail::GetAvailable_impl_throw(
    HSTR_PHYS_DOM in_PhysDomainID,
    HSTR_CPU_MASK out_AvailableCPUmask)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(out_AvailableCPUmask);
    IsInitialized_impl_throw();

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") not found"
                                  );
    }

    hStreams_CPUMask ret = dom->getAvailableStreamCPUMask();
    memcpy(out_AvailableCPUmask, ret.mask, sizeof(HSTR_CPU_MASK));
} // detail::GetAvailable_impl_throw

void
detail::AddLogDomain_impl_throw(
    HSTR_PHYS_DOM      in_PhysDomainID,
    HSTR_CPU_MASK      in_CPUmask,
    HSTR_LOG_DOM      *out_pLogDomainID,
    HSTR_OVERLAP_TYPE *out_pOverlap)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(in_CPUmask);
    HSTR_TRACE_FUN_ARG(out_pLogDomainID);
    HSTR_TRACE_FUN_ARG(out_pOverlap);

    IsInitialized_impl_throw();

#ifdef _WIN32
    if (in_PhysDomainID == HSTR_SRC_PHYS_DOMAIN) {
        uint32_t            NumThreads, CoreMaxMHz;
        uint64_t            SupportedMemTypes, PhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE];
        HSTR_ISA_TYPE       ISA;
        HSTR_CPU_MASK       MaxCPUmask, AvoidCPUmask;

        GetPhysDomainDetails_impl_throw(
            HSTR_SRC_PHYS_DOMAIN,
            &NumThreads,
            &ISA,
            &CoreMaxMHz,
            MaxCPUmask,
            AvoidCPUmask,
            &SupportedMemTypes,
            PhysicalBytesPerMemType);
        if (NumThreads > 64) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_IMPLEMENTED, StringBuilder()
                                       << "Use of the Windows-based host physical domain which "
                                       << "contains more than 64 HW threads is not supported."
                                      );
        }
    }
#endif

    if (out_pLogDomainID  == NULL ||
            out_pOverlap  == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "An output pointer argument of hStreams_AddLogDomain was NULL"
                                  );
    }

    if (0 == HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE, StringBuilder()
                                   << "The CPU mask of the logical domain to be created is empty."
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") not found"
                                  );
    }

    // check the CPU mask - whether it fits in the physical domain's mask
    HSTR_CPU_MASK tmp_cpu_mask;
    HSTR_CPU_MASK_AND(tmp_cpu_mask, phys_dom->getMaxCPUMask().mask, in_CPUmask);


    if (HSTR_CPU_MASK_COUNT(tmp_cpu_mask) < HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE, StringBuilder()
                                   << "The CPU mask of the logical domain to be created is not "
                                   << "contained in the CPU mask of the physical domain."
                                  );
    }

    // check the CPU mask - whether it partially overlaps with some other logical domain
    HSTR_OVERLAP_TYPE overlap;
    phys_dom->lookupLogDomainByCPUMask(in_CPUmask, &overlap);
    if (HSTR_PARTIAL_OVERLAP == overlap) {
        *out_pOverlap = overlap;
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OVERLAPPING_RESOURCES, StringBuilder()
                                   << "The CPU mask of the logical domain to be created "
                                   << "partially overlaps the CPU mask of another logical domain."
                                  );
    }

    const HSTR_LOG_DOM newID = getNextLogDomID();
    HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
            << "Creating new logical domain (ID="
            << newID
            << ")";
    hStreams_LogDomain *log_dom = new hStreams_LogDomain(newID, in_CPUmask, *phys_dom);

    // create physical buffer for each incremental logical buffer
    hStreams_LogBufferCollection::iterator it;
    for (it = log_buffers.begin(); it != log_buffers.end(); ++it) {
        hStreams_LogBuffer *log_buf = *it;
        if (log_buf->isPropertyFlagSet(HSTR_BUF_PROP_INCREMENTAL)) {
            HSTR_RESULT hret = log_buf->attachExistingLogDomain(*log_dom);
            if (hret != HSTR_RESULT_SUCCESS) {
                // Revert changes if any error occurred
                log_buffers.processDelLogDomain(*log_dom);
                delete log_dom;
                throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                           << "Could not instantiate incremental buffer "
                                           << log_buf->getStart()
                                           << " for new logical domain (ID="
                                           << log_dom->id()
                                           << "): "
                                           << hStreams_ResultGetName(hret)
                                          );
            } else {
                HSTR_DEBUG2(HSTR_INFO_TYPE_MISC)
                        << "Successfully created a new instantiation of incremental logical buffer "
                        << log_buf->getStart()
                        << " for new logical domain (ID="
                        << log_dom->id()
                        << ")";
            }
        }
    }

    phys_dom->addLogDomainMapping(*log_dom);
    log_domains.addToCollection(log_dom);

    *out_pOverlap = overlap;
    *out_pLogDomainID = newID;
} // detail::AddLogDomain_impl_throw

void
detail::RmLogDomains_impl_throw(
    uint32_t       in_NumLogDomains,
    HSTR_LOG_DOM  *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);
    IsInitialized_impl_throw();

    if (in_pLogDomainIDs == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_pLogDomainIDs pointer argument of hStreams_RmLogDomains was NULL"
                                  );
    }
    if (in_NumLogDomains == 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_NumLogDomains argument of hStreams_RmLogDomains was equal to 0"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    for (uint32_t log_dom_idx = 0; log_dom_idx < in_NumLogDomains; ++log_dom_idx) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[log_dom_idx]);
        if (NULL == log_dom) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                       << "Logical domain with ID in_pLogDomainIDs["
                                       << log_dom_idx
                                       << "] == "
                                       << in_pLogDomainIDs[log_dom_idx]
                                       << " doesn't exist."
                                      );
        }
        log_streams.delFromCollectionByLogDomain(*log_dom);
        log_dom->destroyAllStreams();
        // Finally, erase the logical domain form the mappings and destroy it
        log_domains.delFromCollection(*log_dom);
        log_dom->getPhysDomain().delLogDomainMapping(*log_dom);
        log_buffers.processDelLogDomain(*log_dom);
        delete log_dom;
    }
}

void
detail::GetNumLogDomains_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t       *out_pNumLogDomains)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(out_pNumLogDomains);
    IsInitialized_impl_throw();

    if (out_pNumLogDomains == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pNumLogDomains pointer argument of hStreams_GetNumPhysDomains was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == phys_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") doesn't exist"
                                  );
    }
    *out_pNumLogDomains = phys_dom->getNumLogDomains();
} // detail::GetNumLogDomains_impl_throw

void
detail::GetLogDomainIDList_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumLogDomains,
    HSTR_LOG_DOM   *out_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_PhysDomainID);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(out_pLogDomainIDs);
    IsInitialized_impl_throw();
    if (out_pLogDomainIDs == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pLogDomainIDs pointer argument of hStreams_GetNumPhysDomains was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_PhysDomain *phys_dom = phys_domains.lookupByPhysDomainID(in_PhysDomainID);
    if (NULL == phys_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Physical domain (ID="
                                   << in_PhysDomainID
                                   << ") doesn't exist"
                                  );
    }

    uint32_t num_written, num_present;
    phys_dom->getLogDomainIDs(out_pLogDomainIDs, in_NumLogDomains, &num_written, &num_present);

    if (num_written < in_NumLogDomains) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Found only "
                                   << num_written
                                   << " logical domains, instead of in_NumLogDomains == "
                                   << in_NumLogDomains
                                  );
    }

    if (num_present > in_NumLogDomains) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "There are "
                                   << num_present
                                   << " logical domains, more than in_NumLogDomains == "
                                   << in_NumLogDomains
                                  );
    }
} // detail::GetLogDomainIDList_impl_throw

void
detail::GetLogDomainDetails_impl_throw(
    HSTR_LOG_DOM   in_LogDomainID,
    HSTR_PHYS_DOM *out_pPhysDomainID,
    HSTR_CPU_MASK  out_CPUmask)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogDomainID);
    HSTR_TRACE_FUN_ARG(out_pPhysDomainID);
    HSTR_TRACE_FUN_ARG(out_CPUmask);
    IsInitialized_impl_throw();

    if (out_pPhysDomainID == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pPhysDomainID pointer argument of hStreams_GetLogDomainDetails was NULL"
                                  );
    }
    // no point in checking out_CPUmask since it's an array, not a pointer

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);
    if (NULL == log_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical domain with ID "
                                   << in_LogDomainID
                                   << " not found"
                                  );
    }

    memcpy(out_CPUmask, log_dom->getCPUMask().mask, sizeof(HSTR_CPU_MASK));
    *out_pPhysDomainID = log_dom->getPhysDomain().id();
} // detail::hStreams_GetLogDomainDetails_impl_throw

void
detail::StreamCreate_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    HSTR_LOG_DOM        in_LogDomainID,
    const HSTR_CPU_MASK in_CPUmask)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_LogDomainID);
    HSTR_TRACE_FUN_ARG(in_CPUmask);
    IsInitialized_impl_throw();

    if (0 == HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE, StringBuilder()
                                   << "An empty CPU maks has been supplied to hStreams_StreamCreate."
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    hStreams_LogStream *lookup_str = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL != lookup_str) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_ALREADY_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " already exists"
                                  );
    }

    hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);
    if (NULL == log_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Logical domain with ID in_LogDomainID == "
                                   << in_LogDomainID
                                   << " doesn't exist"
                                  );
    }

    // check the CPU mask - whether it fits in the logical domain's mask
    HSTR_CPU_MASK tmp_cpu_mask;
    HSTR_CPU_MASK_AND(tmp_cpu_mask, log_dom->getCPUMask().mask, in_CPUmask);
    if (HSTR_CPU_MASK_COUNT(tmp_cpu_mask) < HSTR_CPU_MASK_COUNT(in_CPUmask)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_CPU_MASK_OUT_OF_RANGE, StringBuilder()
                                   << "The CPU mask of the logical stream to be created is not "
                                   << "contained within the CPU mask of the logical domain."
                                  );
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
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                       << "An error occured while trying to create a physical stream on the device"
                                      );
        }
        new_log_stream = new hStreams_LogStream(in_LogStreamID, non_const_cpu_mask, *log_dom, *new_phys_stream);
        // log stream has already attached to the physical stream by itself, we can detach now
        // must be done after hStreams_LogStream initialization with new.
        new_phys_stream->detach();
    }
    if (!new_log_stream) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "An internal error occured while trying to create the logical stream"
                                  );
    }

    log_streams.addToCollection(new_log_stream);
    log_dom->addLogStreamMapping(*new_log_stream);
} // detail::hStreams_StreamCreate_impl_throw

void
detail::StreamDestroy_impl_throw(HSTR_LOG_STR in_LogStreamID)
{
    detail::IsInitialized_impl_throw();
    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    hStreams_LogStream *log_str = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_str) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "The stream to be deleted (ID="
                                   << in_LogStreamID
                                   << ") was not found"
                                  );
    }

    log_streams.delFromCollection(*log_str);
    log_str->getLogDomain().delLogStreamMapping(*log_str);
    delete log_str;
} // detail::StreamDestroy_impl_throw

void
detail::GetNumLogStreams_impl_throw(
    HSTR_LOG_DOM   in_LogDomainID,
    uint32_t      *out_pNumLogStreams)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogDomainID);
    HSTR_TRACE_FUN_ARG(out_pNumLogStreams);
    IsInitialized_impl_throw();

    if (out_pNumLogStreams == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pNumLogStreams pointer argument of hStreams_GetNumLogStreams was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_LogDomainID);
    if (NULL == log_dom) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                   << "Logical domain with id in_LogDomainID == "
                                   << in_LogDomainID
                                   << " was not found"
                                  );
    }
    *out_pNumLogStreams = log_dom->getNumLogStreams();
} // detail::GetNumLogStreams_impl_throw

void
detail::GetLogStreamIDList_impl_throw(
    HSTR_LOG_DOM  in_LogDomainID,
    uint32_t      in_NumLogStreams,
    HSTR_LOG_STR *out_pLogStreamIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogDomainID);
    HSTR_TRACE_FUN_ARG(in_NumLogStreams);
    HSTR_TRACE_FUN_ARG(out_pLogStreamIDs);
    IsInitialized_impl_throw();

    if (out_pLogStreamIDs == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "out_pLogStreamIDs pointer argument of hStreams_GetLogStreamIDList was NULL"
                                  );
    }

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogDomain *log_domain = log_domains.lookupByLogDomainID(in_LogDomainID);
    if (NULL == log_domain) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical domain with ID in_LogDomainID == "
                                   << in_LogDomainID
                                   << " wasn't found"
                                  );
    }

    uint32_t num_written, num_present;
    log_domain->getLogStreamIDs(out_pLogStreamIDs, in_NumLogStreams, &num_written, &num_present);
    if (num_written < in_NumLogStreams) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Only "
                                   << num_written
                                   << " logical streams were found in logical domain "
                                   << in_LogDomainID
                                   << ", less than in_NumLogStreams == "
                                   << in_NumLogStreams
                                  );
    }

    if (num_present > in_NumLogStreams) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "There are  "
                                   << num_present
                                   << " logical streams in logical domain "
                                   << in_LogDomainID
                                   << ", more than in_NumLogStreams == "
                                   << in_NumLogStreams
                                  );
    }
} // detail::GetLogStreamIDList_impl_throw

void
detail::GetLogStreamDetails_impl_throw(
    HSTR_LOG_STR      in_LogStreamID,
    HSTR_CPU_MASK     out_CPUmask)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(out_CPUmask);
    IsInitialized_impl_throw();

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    hStreams_LogStream *log_stream = log_streams.lookupByLogStreamID(in_LogStreamID);
    if (NULL == log_stream) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist "
                                  );
    }

    memcpy(out_CPUmask, log_stream->getCPUMask().mask, sizeof(HSTR_CPU_MASK));
} // detail::GetLogStreamDetails_impl_throw

void
detail::EnqueueCompute_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    const char         *in_pFunctionName,
    uint32_t            in_numScalarArgs,
    uint32_t            in_numHeapArgs,
    uint64_t           *in_pArgs,
    HSTR_EVENT         *out_pEvent,
    void               *out_ReturnValue,
    uint16_t            in_ReturnValueSize)
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
    IsInitialized_impl_throw();

    if (in_pFunctionName == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BAD_NAME, StringBuilder()
                                   << "Function name argument to hStreams_EnqueueCompute was NULL"
                                  );
    }
    if (in_ReturnValueSize > HSTR_RETURN_SIZE_LIMIT) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_ReturnValueSize was larger than HSTR_RETURN_SIZE_LIMIT, "
                                   << HSTR_RETURN_SIZE_LIMIT
                                  );
    }
    if (in_numScalarArgs + in_numHeapArgs && !in_pArgs) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_pArgs cannot be NULL if in_NumScalarArgs != 0 && in_numHeapArgs != 0 "
                                  );
    }
    if (((in_ReturnValueSize > 0)  && (out_ReturnValue == NULL)) ||
            ((in_ReturnValueSize == 0) && (out_ReturnValue != NULL))) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "in_ReturnValueSize must be 0 if out_ReturnValue is NULL and vice versa"
                                  );
    }
    if (strlen(in_pFunctionName) > HSTR_MAX_FUNC_NAME_SIZE - 1) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BAD_NAME, StringBuilder()
                                   << "Sorry, "
                                   << in_pFunctionName
                                   << " exceeds max called function name size of "
                                   << HSTR_MAX_FUNC_NAME_SIZE - 1
                                  );
    }
    uint64_t spt = HSTR_ARGS_SUPPORTED;
    if (spt > HSTR_ARGS_IMPLEMENTED) {
        spt = HSTR_ARGS_IMPLEMENTED;
    }
    uint64_t requestedArgs = (uint64_t)3 + (uint64_t)in_numScalarArgs + (uint64_t)in_numHeapArgs;
    if (requestedArgs > spt) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_TOO_MANY_ARGS, StringBuilder()
                                   << "Sorry, implementation only supports no more than (# scalar + # heap) = "
                                   << spt
                                   << " arguments to streamed functions."
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist "
                                  );
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
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                       << "Did not find a corresponding buffer for in_pArgs["
                                       << i
                                       << "] == "
                                       << (void *)in_pArgs[i]
                                      );
        }
        hStreams_PhysBuffer *phys_buf = log_buf->getPhysBufferForLogDomain(log_domain);
        if (!phys_buf) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                       << "Did not find a buffer instantiation for in_pArgs["
                                       << i
                                       << "] == "
                                       << (void *)in_pArgs[i]
                                       << " on logical domain (ID="
                                       << log_domain.id()
                                       << ")"
                                      );
        }
        buffer_args.push_back(phys_buf);
        // NOTE Those are offsets into the source buffers.
        //      Physical buffers will compensate for eventual sink-side
        //      buffer padding themselves.
        uint64_t sink_offset = addr - (uint64_t)log_buf->getStart();
        buffer_offsets.push_back(sink_offset);
    }

    hStreams_PhysStream &phys_stream = log_stream->getPhysStream();
    HSTR_RESULT hret = phys_stream.enqueueFunction(in_pFunctionName, scalar_args, buffer_args,
                       buffer_offsets, out_ReturnValue, (int16_t) in_ReturnValueSize, out_pEvent);
    if (hret != HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                   << "An error occured while attempting to enqueue function \""
                                   << in_pFunctionName
                                   << "\" in logical stream (ID="
                                   << in_LogStreamID
                                   << ")"
                                  );
    }
} // detail::EnqueueCompute_impl_throw


namespace
{
// This just expects the logical stream and the logical domains to have been
// looked up and the appropriate locks to have been grabbed. It is the common
// part for EnqueueData1D and EnqueueDataXDomain1D
void
EnqueueDataXDomain1D_worker_locked_throw(
    hStreams_LogStream &in_LogStream,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    hStreams_LogDomain &in_dstLogDomain,
    hStreams_LogDomain &in_srcLogDomain,
    HSTR_EVENT         *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStream.id());
    HSTR_TRACE_FUN_ARG(in_pWriteAddr);
    HSTR_TRACE_FUN_ARG(in_pReadAddr);
    HSTR_TRACE_FUN_ARG(in_size);
    HSTR_TRACE_FUN_ARG(in_dstLogDomain.id());
    HSTR_TRACE_FUN_ARG(in_srcLogDomain.id());
    HSTR_TRACE_FUN_ARG(out_pEvent);

    if (in_pWriteAddr == NULL || in_pReadAddr == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_pWriteAddr or in_pReadAddr was NULL"
                                  );
    }
    if (0 == in_size) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_size cannot be equal to 0"
                                  );
    }

    if (in_LogStream.getLogDomain().id() != in_srcLogDomain.id() &&
            in_LogStream.getLogDomain().id() != in_dstLogDomain.id()) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "The logical stream does not belong in any of the specified domains."
                                  );
    }

    uint64_t src_u64 = (uint64_t)in_pReadAddr;
    uint64_t dst_u64 = (uint64_t)in_pWriteAddr;

    if (in_srcLogDomain.id() == in_dstLogDomain.id()) {
        // Note for future: also for aliased buffers
        // do arithmetic on a pointer to byte-sized type as the transfers are with that granularity
        if (add_gt(src_u64, in_size, dst_u64) || add_gt(dst_u64, in_size, src_u64)) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                       << "Overlapping transfers within the same logical domain are not permitted."
                                      );
        }
    }

    // implementation note: though we _could_ use lookupLogBuffer(start, size,
    // overlap), the error codes exposed by the EnqueueData* APIs require that
    // HSTR_RESULT_OUT_OF_RANGE be returned if "in_size extends past the end of
    // an allocated buffer". So we have to do the checking by hand here :/

    hStreams_LogBuffer *dst_log_buf = log_buffers.lookupLogBuffer(in_pWriteAddr);
    if (NULL == dst_log_buf) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find the destination buffer."
                                  );
    }
    if (add_gt(dst_u64, in_size, dst_log_buf->getStartu64() + dst_log_buf->getLen())) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Transfer range extends past the end of the destination buffer."
                                  );
    }

    hStreams_PhysBuffer *dst_phys_buf = dst_log_buf->getPhysBufferForLogDomain(in_dstLogDomain);
    if (NULL == dst_phys_buf) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find an instantiation of the destination buffer for logical domain #"
                                   << in_dstLogDomain.id()
                                  );
    }

    hStreams_LogBuffer *src_log_buf = log_buffers.lookupLogBuffer(in_pReadAddr);
    if (NULL == src_log_buf) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find the source buffer."
                                  );
    }
    if (add_gt(src_u64, in_size, src_log_buf->getStartu64() + src_log_buf->getLen())) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Transfer range extends past the end of the source buffer."
                                  );
    }

    hStreams_PhysBuffer *src_phys_buf = src_log_buf->getPhysBufferForLogDomain(in_srcLogDomain);
    if (NULL == src_phys_buf) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find an instantiation of the source buffer for logical domain #"
                                   << in_srcLogDomain.id()
                                  );
    }

    hStreams_PhysStream &phys_stream = in_LogStream.getPhysStream();

    uint64_t dst_offset = (uint64_t)in_pWriteAddr - dst_log_buf->getStartu64();
    uint64_t src_offset = (uint64_t)in_pReadAddr - src_log_buf->getStartu64();

    HSTR_RESULT hret = phys_stream.enqueueTransfer(*dst_phys_buf, *src_phys_buf, dst_offset,
                       src_offset, in_size, out_pEvent);
    if (hret != HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                   << "An error occured while attempting to enqueue the transfer in logical stream (ID="
                                   << in_LogStream.id()
                                   << ")"
                                  );
    }
} // EnqueueDataXDomain1D_worker_locked_throw
} // anonymous namespace


void
detail::EnqueueData1D_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_pWriteAddr);
    HSTR_TRACE_FUN_ARG(in_pReadAddr);
    HSTR_TRACE_FUN_ARG(in_size);
    HSTR_TRACE_FUN_ARG(in_XferDirection);
    HSTR_TRACE_FUN_ARG(out_pEvent);
    IsInitialized_impl_throw();

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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist"
                                  );
    }
    hStreams_LogDomain &other_log_domain = log_stream->getLogDomain();
    hStreams_LogDomain *src_log_domain = log_domains.lookupByLogDomainID(HSTR_SRC_LOG_DOMAIN);
    // Let's be paranoid together
    if (NULL == src_log_domain) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "The logical domain of origin for the buffer's transfer doesn't exist"
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Invalid value for in_XferDirection: "
                                   << in_XferDirection
                                   << ". Valid values are: HSTR_SRC_TO_SINK ("
                                   << HSTR_SRC_TO_SINK
                                   << ") and HSTR_SINK_TO_SRC ("
                                   << HSTR_SINK_TO_SRC
                                   << ")."
                                  );
    } // switch (in_XferDirection)

    EnqueueDataXDomain1D_worker_locked_throw(*log_stream, in_pWriteAddr, in_pReadAddr,
            in_size, *xfer_dst_log_domain, *xfer_src_log_domain, out_pEvent);
} // detail::EnqueueData1D_impl_throw

void
detail::EnqueueDataXDomain1D_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_LOG_DOM        in_dstLogDomain,
    HSTR_LOG_DOM        in_srcLogDomain,
    HSTR_EVENT         *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_pWriteAddr);
    HSTR_TRACE_FUN_ARG(in_pReadAddr);
    HSTR_TRACE_FUN_ARG(in_size);
    HSTR_TRACE_FUN_ARG(in_dstLogDomain);
    HSTR_TRACE_FUN_ARG(in_srcLogDomain);
    HSTR_TRACE_FUN_ARG(out_pEvent);
    IsInitialized_impl_throw();

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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist"
                                  );
    }

    hStreams_LogDomain *xfer_dst_log_domain = log_domains.lookupByLogDomainID(in_dstLogDomain);
    if (NULL == xfer_dst_log_domain) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Transfer destination logical domain (ID="
                                   << in_dstLogDomain
                                   << ") doesn't exist"
                                  );
    }

    hStreams_LogDomain *xfer_src_log_domain = log_domains.lookupByLogDomainID(in_srcLogDomain);
    if (NULL == xfer_src_log_domain) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Transfer source logical domain (ID="
                                   << in_srcLogDomain
                                   << ") doesn't exist"
                                  );
    }
    EnqueueDataXDomain1D_worker_locked_throw(*log_stream, in_pWriteAddr, in_pReadAddr, in_size,
            *xfer_dst_log_domain, *xfer_src_log_domain, out_pEvent);
} // detail::EnqueueDataXDomain1D_impl_throw

void
detail::StreamSynchronize_impl_throw(HSTR_LOG_STR in_LogStreamID)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    IsInitialized_impl_throw();

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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist"
                                  );
    }

    std::vector<HSTR_EVENT> the_events;
    log_stream->getAllEvents(the_events);

    if (the_events.empty()) {
        // nothing to do
        return;
    }

    int32_t timeout = (int32_t)hStreams_GetOptions_time_out_ms_val();
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait((int16_t) the_events.size(),
                            &the_events[0], timeout, true, NULL, NULL);

    if (result == HSTR_COI_SUCCESS) {
        return;
    } else if (result == HSTR_COI_PROCESS_DIED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                   << "Sink-side process died."
                                  );
    } else if (result == HSTR_COI_EVENT_CANCELED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_EVENT_CANCELED, StringBuilder()
                                   << "At least one of the events has been cancelled."
                                  );
    } else if (result == HSTR_COI_TIME_OUT_REACHED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_TIME_OUT_REACHED, StringBuilder()
                                   << "Timeout value of "
                                   << timeout
                                   << " was not large enough, you might wish to adjust the value of "
                                   << "time_out_ms_val in HSTR_OPTIONS"
                                  );
    } else {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Internal error, COI returned an unexpected value: "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
    }
} // detail::StreamSynchronize_impl_throw(HSTR_LOG_STR in_LogStreamID)

void
detail::ThreadSynchronize_impl_throw()
{
    HSTR_TRACE_FUN_ENTER();
    IsInitialized_impl_throw();

    hStreams_RW_Scope_Locker_Unlocker phys_domains_scope_lock(phys_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    hStreams_RW_Scope_Locker_Unlocker log_domains_scope_lock(log_domains_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    // See StreamSynchronize_impl_throw for detailed explanation about the write lock
    hStreams_RW_Scope_Locker_Unlocker log_streams_scope_lock(log_streams_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    hStreams_RW_Scope_Locker_Unlocker log_buffers_scope_lock(log_buffers_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    std::vector<HSTR_EVENT> all_the_events;
    log_streams.getEventsFromAllStreams(all_the_events);

    if (all_the_events.empty()) {
        // nothing to be done
        return;
    }

    int32_t timeout = (int32_t)hStreams_GetOptions_time_out_ms_val();
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait((int16_t) all_the_events.size(),
                            &all_the_events[0], timeout, true, NULL, NULL);
    if (result == HSTR_COI_SUCCESS) {
        return;
    } else if (result == HSTR_COI_PROCESS_DIED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                   << "Sink-side process died."
                                  );
    } else if (result == HSTR_COI_EVENT_CANCELED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_EVENT_CANCELED, StringBuilder()
                                   << "At least one of the events has been cancelled."
                                  );
    } else if (result == HSTR_COI_TIME_OUT_REACHED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_TIME_OUT_REACHED, StringBuilder()
                                   << "Timeout value of "
                                   << timeout
                                   << " was not large enough, you might wish to adjust the value of "
                                   << "time_out_ms_val in HSTR_OPTIONS"
                                  );
    } else {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Internal error, COI returned an unexpected value: "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
    }
} // detail::ThreadSynchronize_impl_throw()

void
detail::EventWait_impl_throw(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    bool               in_WaitForAll,
    int32_t            in_TimeOutMilliSeconds,
    uint32_t          *out_pNumSignaled,
    uint32_t          *out_pSignaledIndices)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_NumEvents);
    HSTR_TRACE_FUN_ARG(in_pEvents);
    HSTR_TRACE_FUN_ARG(in_WaitForAll);
    HSTR_TRACE_FUN_ARG(in_TimeOutMilliSeconds);
    HSTR_TRACE_FUN_ARG(out_pNumSignaled);
    HSTR_TRACE_FUN_ARG(out_pSignaledIndices);
    IsInitialized_impl_throw();

    if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_NONE) {
        HSTR_LOG(HSTR_INFO_TYPE_SYNC) << "Dependencies ignored in EventWait";
        return;
    }

    // Do a COIEventWait on it
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait(
                                (uint16_t)in_NumEvents,
                                in_pEvents,
                                in_TimeOutMilliSeconds,
                                in_WaitForAll,
                                out_pNumSignaled,
                                out_pSignaledIndices);

    if (result == HSTR_COI_SUCCESS) {
        return;
    } else if (result == HSTR_COI_PROCESS_DIED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                   << "Sink-side process died."
                                  );
    } else if (result == HSTR_COI_EVENT_CANCELED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_EVENT_CANCELED, StringBuilder()
                                   << "At least one of the events has been cancelled."
                                  );
    } else if (result == HSTR_COI_TIME_OUT_REACHED) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_TIME_OUT_REACHED, StringBuilder()
                                   << "Waiting on events timeouted"
                                  );
    } else if (result == HSTR_COI_OUT_OF_RANGE) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE);
    } else if (result == HSTR_COI_INVALID_POINTER) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "A required pointer was NULL in hStreams_EventWait"
                                  );
    } else if (result == HSTR_COI_ARGUMENT_MISMATCH) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS);
    } else {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Internal error, COI returned an unexpected value: "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
    }
} // detail::EventWait_impl_throw

void
detail::EventStreamWait_impl_throw(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT        *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_NumEvents);
    HSTR_TRACE_FUN_ARG(in_pEvents);
    HSTR_TRACE_FUN_ARG(in_NumAddresses);
    HSTR_TRACE_FUN_ARG(in_pAddresses);
    HSTR_TRACE_FUN_ARG(out_pEvent);
    IsInitialized_impl_throw();

    if (((in_NumEvents == 0) != (in_pEvents == NULL)) || // can't be NULL if > 0
            ((in_NumAddresses <= 0) != (in_pAddresses == NULL)) || // can't be NULL if > 0
            ((in_NumAddresses == HSTR_WAIT_NONE) && (out_pEvent == NULL))) { // can't be NULL if no deps
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Inconsistent arguments were provided to hStreams_EventStreamWait"
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Logical stream with ID "
                                   << in_LogStreamID
                                   << " doesn't exist."
                                  );
    }

    hStreams_PhysStream &phys_stream = log_stream->getPhysStream();
    hStreams_LogDomain &log_domain = log_stream->getLogDomain();

    std::vector<hStreams_PhysBuffer *> phys_buffers;
    if (in_NumAddresses > 0) {
        phys_buffers.reserve(in_NumAddresses);
        for (int32_t i = 0; i < in_NumAddresses; ++i) {
            hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_pAddresses[i]);
            if (NULL == log_buf) {
                throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                           << "Did not find a corresponding logical buffer for in_pAddresses["
                                           << i
                                           << "] == "
                                           << in_pAddresses[i]
                                          );
            }
            hStreams_PhysBuffer *phys_buf = log_buf->getPhysBufferForLogDomain(log_domain);
            if (NULL == phys_buf) {
                throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                           << "Did not find an instantiation of the logical buffer for in_pAddresses["
                                           << i
                                           << "] == "
                                           << in_pAddresses[i]
                                           << " in logical domain (ID="
                                           << log_domain.id()
                                           << ")"
                                          );
            }
            phys_buffers.push_back(phys_buf);
        }
    }

    if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_NONE) {
        HSTR_LOG(HSTR_INFO_TYPE_SYNC) << "Dependencies ignored in EventStreamWait";
        return;
    }

    // If no events or a control dep follows, make depend on all preceding
    // If deps on all previous were to not be enforced in the
    //  in_NumAddresses == HSTR_WAIT_CONTROL case,  previously-specified
    //  dependences would be lost: we'd be depending only on the wait's predecessors
    DEP_TYPE dep_type = (in_NumEvents == HSTR_WAIT_CONTROL ||
                         in_NumAddresses == HSTR_WAIT_CONTROL)
                        ? IS_BARRIER : IS_XFER;
    HSTR_DEBUG1(HSTR_INFO_TYPE_SYNC)
            << "Input dependency type in hStreams_EventStreamWait: "
            << (dep_type == IS_BARRIER) ? "barrier" : "transfer";

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
                                HSTR_COI_COPY_USE_CPU,                  // in_Type
                                (uint32_t) events.size(),               // total number of dependencies
                                (events.empty()) ? NULL : &events[0],   // in_pDependencies
                                &completion);                           // out_pCompletion

    // Result checking
    if (result != HSTR_COI_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_REMOTE_ERROR, StringBuilder()
                                   << "A problem occured while gathering the dependencies in "
                                   << "hStreams_EventStreamWait. COIBufferWrite returned: "
                                   << hStreams_COIWrapper::COIResultGetName(result)
                                  );
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
        HSTR_DEBUG1(HSTR_INFO_TYPE_SYNC)
                << "Output dependency type in hStreams_EventStreamWait: barrier";
    } else if (in_NumAddresses == HSTR_WAIT_NONE) {
        dep_type = NONE;
        HSTR_DEBUG1(HSTR_INFO_TYPE_SYNC)
                << "Output dependency type in hStreams_EventStreamWait: none";
    } else {
        dep_type = IS_XFER;
        HSTR_DEBUG1(HSTR_INFO_TYPE_SYNC)
                << "Output dependency type in hStreams_EventStreamWait: transfer";
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
} // detail::EventStreamWait_impl_throw(

void
detail::Alloc1DEx_impl_throw(
    void                    *in_BaseAddress,
    uint64_t                 in_Size,
    const HSTR_BUFFER_PROPS *in_pBufferProps,
    int64_t                  in_NumLogDomains,
    HSTR_LOG_DOM            *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_BaseAddress);
    HSTR_TRACE_FUN_ARG(in_Size);
    HSTR_TRACE_FUN_ARG(in_pBufferProps);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);
    IsInitialized_impl_throw();

    // Use default Alloc1DEx properties object if in_pBufferProps == NULL
    HSTR_BUFFER_PROPS default_props = HSTR_BUFFER_PROPS_INITIAL_VALUES_EX;
    if (in_pBufferProps == NULL) {
        HSTR_DEBUG1(HSTR_INFO_TYPE_MEM)
                << "Using default buffer properties in hStreams_Alloc1DEx";
        in_pBufferProps = &default_props;
    } else if (in_pBufferProps->mem_type != HSTR_MEM_TYPE_NORMAL ||
               (in_pBufferProps->flags & HSTR_BUF_PROP_AFFINITIZED) != 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_IMPLEMENTED, StringBuilder()
                                   << "Memory type other than normal or buffer affinitization "
                                   << "was requested in hStreams_Alloc1DEx, both of which are "
                                   << "not implemented yet"
                                  );
    }

    if (in_Size == 0 || in_NumLogDomains < -1) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_Size was equal to 0 or in_NumLogDomains < -1"
                                  );
    }
    if (addition_overflow((uint64_t)in_BaseAddress, in_Size)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_Size value is too large"
                                  );
    }
    if (in_BaseAddress == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_BaseAddress cannot be NULL"
                                  );
    }
    if (((in_NumLogDomains == -1 || in_NumLogDomains == 0) && (in_pLogDomainIDs != NULL))
            || (!(in_NumLogDomains == -1 || in_NumLogDomains == 0) && (in_pLogDomainIDs == NULL))) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Inconsistent arguments were passed to the allocation API"
                                  );
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "The in_pLogDomainIDs array must not include HSTR_SRC_LOG_DOMAIN"
                                      );
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

    std::unordered_set<hStreams_LogDomain *> log_domains_set;

    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "Logical domain with id in_pLogDomainIDs["
                                       << i
                                       << "] == "
                                       << in_pLogDomainIDs[i]
                                       << " doesn't exist"
                                      );
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "Values in the in_pLogDomainIDs array must not repeat"
                                      );
        }
    }

    HSTR_OVERLAP_TYPE buf_overlap = HSTR_NO_OVERLAP;
    log_buffers.lookupLogBuffer(in_BaseAddress, in_Size, &buf_overlap);
    if (HSTR_EXACT_OVERLAP == buf_overlap) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_ALREADY_FOUND, StringBuilder()
                                   << "Buffer "
                                   << in_BaseAddress
                                   << " already exists"
                                  );
    }
    if (HSTR_PARTIAL_OVERLAP == buf_overlap) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OVERLAPPING_RESOURCES, StringBuilder()
                                   << "Buffers must not overlap."
                                  );
    }

    hStreams_LogBuffer *log_buf = new hStreams_LogBuffer(in_BaseAddress, in_Size,
            *in_pBufferProps);

    // Attach all selected logical domains
    if (in_NumLogDomains == -1) {
        HSTR_RESULT hret = log_buf->attachLogDomainFromRange(
                               log_domains.begin(), log_domains.end());
        if (hret != HSTR_RESULT_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                       << "An error was encountered while instantiating buffer "
                                       << in_BaseAddress
                                      );
        }
    } else {
        // Buffer will always be created for HSTR_SRC_LOG_DOMAIN, and it can't be in in_pLogDomainIDs
        // so it is added manually here
        hStreams_LogDomain *src_log_dom = log_domains.lookupByLogDomainID(HSTR_SRC_LOG_DOMAIN);
        log_domains_set.insert(src_log_dom);
        HSTR_RESULT hret = log_buf->attachLogDomainFromRange(
                               log_domains_set.begin(), log_domains_set.end());
        if (hret != HSTR_RESULT_SUCCESS) {
            throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                       << "An error was encountered while instantiating buffer "
                                       << in_BaseAddress
                                      );
        }
    }

    log_buffers.addToCollection(log_buf);
} // detail::Alloc1DEx_impl_throw


void
detail::AddBufferLogDomains_impl_throw(
    void            *in_Address,
    uint64_t         in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);
    IsInitialized_impl_throw();

    if (in_NumLogDomains == 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_NumLogDomains cannot be equal to 0"
                                  );
    }
    if ((in_Address == NULL) || (in_pLogDomainIDs == NULL)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Either the buffer address or logical domains array"
                                   << "was NULL in hStreams_AddBufferLogdomains"
                                  );
    }
    for (uint64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "The logical domains array must not contain HSTR_SRC_LOG_DOMAIN"
                                      );
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

    std::unordered_set<hStreams_LogDomain *> log_domains_set;
    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);

    if (log_buf == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer for "
                                   << in_Address
                                  );
    }
    for (uint64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "Did not find logical domain in_pLogDomainIDs["
                                       << i
                                       << "]: "
                                       << in_pLogDomainIDs[i]
                                      );
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "The in_pLogDomainIDs array must not contain duplicate entries"
                                      );
        }

        if (log_buf->getPhysBufferForLogDomain(*log_dom) != NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_ALREADY_FOUND, StringBuilder()
                                       << "Logical buffer "
                                       << log_buf->getStart()
                                       << "already has an instantiation in logical domain #"
                                       << log_dom->id()
                                      );
        }
    }

    // Attach buffer for selected logical domains
    HSTR_RESULT hret = log_buf->attachLogDomainFromRange(
                           log_domains_set.begin(), log_domains_set.end());
    if (hret != HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(hret, StringBuilder()
                                   << "An error was encountered while instantiating buffer "
                                   << log_buf->getStart()
                                   << " in additional logical domains"
                                  );
    }
} // detail::AddBufferLogDomains_impl_throw

void
detail::RmBufferLogDomains_impl_throw(
    void               *in_Address,
    int64_t             in_NumLogDomains,
    HSTR_LOG_DOM       *in_pLogDomainIDs)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pLogDomainIDs);
    IsInitialized_impl_throw();

    if (in_Address == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_Address cannot be equal NULL"
                                  );
    }
    if (in_NumLogDomains == 0 || in_NumLogDomains < -1) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_NumLogDomains cannot be < -1"
                                  );
    }
    if (((in_NumLogDomains == - 1) && (in_pLogDomainIDs != NULL))
            || ((in_NumLogDomains != - 1) && (in_pLogDomainIDs == NULL))) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "The arguments of hStreams_RmBufferLogDomains were inconsistent"
                                  );
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        if (in_pLogDomainIDs[i] == HSTR_SRC_LOG_DOMAIN) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "The logical domains array must not contain HSTR_SRC_LOG_DOMAIN"
                                      );
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

    std::unordered_set<hStreams_LogDomain *> log_domains_set;
    hStreams_LogBuffer *log_buf = log_buffers.lookupLogBuffer(in_Address);

    if (log_buf == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer containing the address "
                                   << in_Address
                                  );
    }
    for (int64_t i = 0; i < in_NumLogDomains; i++) {
        hStreams_LogDomain *log_dom = log_domains.lookupByLogDomainID(in_pLogDomainIDs[i]);
        if (log_dom == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_DOMAIN_OUT_OF_RANGE, StringBuilder()
                                       << "Did not find a logical domain with ID in_pLogDomainIDs["
                                       << i
                                       << "] == "
                                       << in_pLogDomainIDs[i]
                                      );
        }

        if (log_domains_set.find(log_dom) == log_domains_set.end()) {
            log_domains_set.insert(log_dom);
        } else {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "The in_pLogDomainIDs array must not contain duplicate entries"
                                      );
        }

        if (log_buf->getPhysBufferForLogDomain(*log_dom) == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                       << "Did not find an instantiation of the logical buffer "
                                       << log_buf->getStart()
                                       << "for logical domain #"
                                       << log_dom->id()
                                      );
        }
    }

    // Detach buffer from selected logical domains
    if (in_NumLogDomains == -1) {
        log_buf->detachAllLogDomain();
    } else {
        log_buf->detachLogDomainFromRange(log_domains_set.begin(), log_domains_set.end());
    }
} // detail::RmBufferLogDomains_impl_throw

void
detail::GetBufferNumLogDomains_impl_throw(
    void                *in_Address,
    uint64_t            *out_NumLogDomains)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(out_NumLogDomains);
    IsInitialized_impl_throw();

    if (in_Address == NULL || out_NumLogDomains == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Either the buffer address or output pointer operand "
                                   << "was NULL in hStreams_GetBufferNumLogDomains"
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer containing the address "
                                   << in_Address
                                  );
    }
    *out_NumLogDomains = log_buf->getNumAttachedLogDomains();
} // detail::GetBufferNumLogDomains_impl_throw


void
detail::GetBufferLogDomains_impl_throw(
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
    IsInitialized_impl_throw();

    if (in_Address == NULL || out_pNumLogDomains == NULL || out_pLogDomains == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Either the buffer address or output pointer operand "
                                   << "was NULL in hStreams_GetBufferLogDomains"
                                  );
    }
    if (in_NumLogDomains == 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_NumLogDomains must be > 0"
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer containing the address "
                                   << in_Address
                                  );
    }

    log_buf->getAttachedLogDomainIDs(in_NumLogDomains, out_pLogDomains, out_pNumLogDomains);

    if (in_NumLogDomains < *out_pNumLogDomains) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "The buffer is instantiated in "
                                   << *out_pNumLogDomains
                                   << " logical domains, which is more than in_NumLogDomains == "
                                   << in_NumLogDomains
                                  );
    }
} // detail::GetBufferLogDomains_impl_throw

void
detail::GetBufferProps_impl_throw(
    void                *in_Address,
    HSTR_BUFFER_PROPS   *out_BufferProps)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);
    HSTR_TRACE_FUN_ARG(out_BufferProps);
    IsInitialized_impl_throw();

    if (in_Address == NULL || out_BufferProps == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Either the buffer address or output pointer operand "
                                   << "was NULL in hStreams_GetBufferLogDomains"
                                  );
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
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer containing the address "
                                   << in_Address
                                  );
    }
    *out_BufferProps = log_buf->getProperties();
} // detail::GetBufferProps_impl_throw

void
detail::DeAlloc_impl_throw(void *in_Address)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_Address);

    IsInitialized_impl_throw();
    if (!in_Address) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_Address cannot be NULL for deallocating the buffer"
                                  );
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
    if (log_buf == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_FOUND, StringBuilder()
                                   << "Did not find a logical buffer containing the address "
                                   << in_Address
                                  );
    }
    log_buffers.delFromCollection(*log_buf);
    delete log_buf;
} // detail::DeAlloc_impl_throw(void *in_Address)


void
detail::Cfg_SetLogLevel(HSTR_LOG_LEVEL in_loglevel)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_loglevel);

    if (IsInitialized_impl_nothrow() == HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_PERMITTED, StringBuilder()
                                   << "hStreams_Cfg_SetLogLevel() cannot "
                                   << "be called if the library has been already initialized."
                                  );
    }
    if (in_loglevel < HSTR_LOG_LEVEL_NO_LOGGING
            || in_loglevel > HSTR_LOG_LEVEL_DEBUG4) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Invalid value for the logging level"
                                  );
    }
    globals::logging_level = in_loglevel;
} // detail::Cfg_SetLogLevel(HSTR_LOG_LEVEL in_loglevel)

void
detail::Cfg_SetLogInfoType(uint64_t in_info_type_mask)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_info_type_mask);

    if (IsInitialized_impl_nothrow() == HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_PERMITTED, StringBuilder()
                                   << "hStreams_Cfg_SetLogInfoType() cannot "
                                   << "be called if the library has been already initialized."
                                  );
    }
    globals::logging_bitmask = in_info_type_mask;
} // detail::Cfg_SetLogInfoType(uint64_t in_info_type_mask)

void
detail::Cfg_SetMKLInterface(HSTR_MKL_INTERFACE in_MKLInterface)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_MKLInterface);
    if ((in_MKLInterface < 0) || (HSTR_MKL_INTERFACE_SIZE <= in_MKLInterface)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Invalid value for the MKL interface"
                                  );
    }
    if (IsInitialized_impl_nothrow() == HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_PERMITTED, StringBuilder()
                                   << "hStreams_Cfg_SetMKLInterface() cannot "
                                   << "be called if the library has been already initialized."
                                  );
    }
    globals::mkl_interface = in_MKLInterface;
} // detail::Cfg_SetMKLInterface(HSTR_MKL_INTERFACE in_MKLInterface)

void
detail::GetCurrentOptions_impl_throw(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(pCurrentOptions);
    HSTR_TRACE_FUN_ARG(buffSize);
    if (pCurrentOptions == NULL || buffSize < sizeof(HSTR_OPTIONS)) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Bad arguments to hStreams_GetCurrentOptions, "
                                   << "NULL output pointer or too small buffer"
                                  );
    }
    hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(globals::options_lock,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);

    *pCurrentOptions = globals::options;
} // detail::GetCurrentOptions_impl_throw(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize)

namespace
{
// This expects arguments are validated.
// It is the common part for SetOptions and SetLibrariesToLoad
void SetLibrariesToLoad_worker(
    HSTR_ISA_TYPE in_isaType,
    uint32_t in_NumLibNames,
    char **in_ppLibNames,
    int *in_pLibFlags
)
{
    hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(
        globals::libraries_to_load_lock, hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);

    std::vector<std::pair<std::string, int>> lib_names_with_flags;
    lib_names_with_flags.reserve(in_NumLibNames);

    for (uint32_t i = 0; i < in_NumLibNames; i++) {
        std::pair<std::string, int> single_lib_with_flags(
            std::string(in_ppLibNames[i]),
            (in_pLibFlags != NULL ? in_pLibFlags[i] : HSTR_COI_LOADLIBRARY_V1_FLAGS));
        lib_names_with_flags.push_back(single_lib_with_flags);
    }

    globals::libraries_to_load[in_isaType] = lib_names_with_flags;

} // SetLibrariesToLoad_worker
} // anonymous namespace

void
detail::SetOptions_impl_throw(const HSTR_OPTIONS *in_options)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_options);
    if (in_options == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Options cannot be NULL"
                                  );
    }
    if (in_options->_hStreams_FatalError == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "_hStreams_FatalError cannot be NULL"
                                  );
    }
    if (in_options->kmp_affinity < 0 || in_options->kmp_affinity >= HSTR_KMP_AFFINITY_SIZE) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Incorrect value of kmp_affinity"
                                  );
    }
    if (in_options->dep_policy < 0 || in_options->dep_policy >= HSTR_DEP_POLICY_SIZE) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Incorrect value of dep_policy"
                                  );
    }
    if (in_options->openmp_policy < 0 || in_options->openmp_policy >= HSTR_OPENMP_POLICY_SIZE) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Incorrect value of openmp_policy"
                                  );
    }
    if (in_options->time_out_ms_val <= 0 && in_options->time_out_ms_val != HSTR_TIME_INFINITE) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "Incorrect value of time_out_ms_val"
                                  );
    }
    if (in_options->libNameCnt == 0 && in_options->libNames != NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "libNames must be NULL, if libNameCnt is zero."
                                  );
    }
    if (in_options->libNameCnt != 0 && in_options->libNames == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "libNames can't be NULL, if libNameCnt isn't zero."
                                  );
    }
    for (int i = 0; i < in_options->libNameCnt; i++) {
        if (in_options->libNames[i] == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "libNames can't contain NULL entries."
                                      );
        }
    }
    if (in_options->libNameCntHost == 0 && in_options->libNamesHost != NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "libNamesHost must be NULL, if libNameCntHost is zero."
                                  );
    }
    if (in_options->libNameCntHost != 0 && in_options->libNamesHost == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                   << "libNamesHost can't be NULL, if libNameCntHost isn't zero."
                                  );
    }
    for (int i = 0; i < in_options->libNameCntHost; i++) {
        if (in_options->libNamesHost[i] == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "libNamesHost can't contain NULL entries."
                                      );
        }
    }

    hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(globals::options_lock, hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    globals::options = *in_options;

    // Check if libNameCntHost or libNameCnt is zero to prevent accidental removal of the
    // libraries added by hStreams_SetLibrariesToLoad()
    if (0 != in_options->libNameCntHost) {
        SetLibrariesToLoad_worker(HSTR_ISA_x86_64, in_options->libNameCntHost, in_options->libNamesHost, NULL);
    }
    if (0 != in_options->libNameCnt) {
        SetLibrariesToLoad_worker(HSTR_ISA_KNC, in_options->libNameCnt, in_options->libNames, in_options->libFlags);
    }
} // detail::SetOptions_impl_throw(const HSTR_OPTIONS *in_options)

void
detail::SetLibrariesToLoad_impl_throw(
    HSTR_ISA_TYPE in_isaType,
    uint32_t in_NumLibNames,
    char **in_ppLibNames,
    int *in_pLibFlags
)
{
    if (IsInitialized_impl_nothrow() == HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_PERMITTED, StringBuilder()
                                   << "hStreams_SetLibrariesToLoad() cannot "
                                   << "be called if the library has been already initialized."
                                  );
    }
    if (0 > in_isaType
            || HSTR_ISA_INVALID == in_isaType
            || HSTR_ISA_MIC == in_isaType
            // Assume that HSTR_ISA_KNL is last valid value, this can be changed in future
            || HSTR_ISA_KNL < in_isaType) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Not supported or invalid in_isaType."
                                  );
    }
    if (0 == in_NumLibNames) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "in_NumLibNames can't be 0."
                                  );
    }
    if (NULL == in_ppLibNames) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_ppLibNames can't be NULL."
                                  );
    }
    for (uint32_t i = 0; i < in_NumLibNames; i++) {
        if (in_ppLibNames[i] == NULL) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "in_ppLibNames can't contain NULL entries."
                                      );
        }
    }
    if (HSTR_ISA_x86_64 == in_isaType && NULL != in_pLibFlags) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NOT_IMPLEMENTED, StringBuilder()
                                   << "Using non-null in_pLibFlags for host in unsupported."
                                  );
    }
    SetLibrariesToLoad_worker(in_isaType, in_NumLibNames, in_ppLibNames, in_pLibFlags);
} // detail::SetLibrariesToLoad_impl_throw

void
detail::GetVersionStringLen_impl_throw(uint32_t *out_pVersionStringLen)
{
    if (NULL == out_pVersionStringLen) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "An output pointer argument of hStreams_GetVersionStringLen was NULL"
                                  );
    }
    *out_pVersionStringLen = (uint32_t)strlen(library_version_string) + 1; // include null termination
} // detail::GetVersionStringLen_impl_throw(uint32_t *out_pVersionStringLen)

void
detail::Version_impl_throw(char *buff, uint32_t buffLength)
{
    if (NULL == buff) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "An output pointer argument of hStreams_Version was NULL"
                                  );
    }

    if (buffLength < (uint32_t)strlen(library_version_string) + 1) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_BUFF_TOO_SMALL, StringBuilder()
                                   << "The output buffer in hStreams_Version was too small"
                                  );
    }
    memcpy(buff, library_version_string, strlen(library_version_string) + 1);
} // detail::Version_impl_throw(char *buff, uint32_t buffLength)
