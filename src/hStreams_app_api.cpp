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

#include <hStreams_app_api.h>
#include <hStreams_source.h>
#include <hStreams_common.h>
#include <hStreams_internal.h>
#include "hStreams_internal_vars_source.h"
#include <hStreams_Logger.h>
#include "hStreams_helpers_common.h"
#include "hStreams_Logger.h"

#include <numeric>

extern "C" {

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_init_domains , 1)(
        uint32_t     in_NumLogDomains,
        uint32_t    *in_pStreamsPerDomain,
        uint32_t     in_LogStreamOversubscription)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_NumLogDomains);
        HSTR_TRACE_FUN_ARG(in_pStreamsPerDomain);
        HSTR_TRACE_FUN_ARG(in_LogStreamOversubscription);

        // FIXME LATER: This assumes that the host is doing offload

        // Error check before calling Init()
        if (in_pStreamsPerDomain == NULL) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE) << "in_pStreamsPerDomain cannot be NULL.";

            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }
        if (!in_NumLogDomains) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE) << "Initializing hStreams with 0 logical domains is prohibited.";

            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }
        if (!in_LogStreamOversubscription) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "Initializing hStreams with 0 logical streams per physical stream is prohibited.";

            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }

        bool is_first_app_initialization;
        // verify if this is the first time the App-API level initialization functions are being called
        if (globals::app_init_log_doms_IDs.empty()) {
            is_first_app_initialization = true;
            HSTR_DEBUG1(HSTR_INFO_TYPE_TRACE) << "First call to and App API-level initialization function.";
        } else {
            is_first_app_initialization = false;
            if (in_NumLogDomains != globals::app_init_log_doms_IDs.size()) {
                HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                        << "The requested number of logical domains to be used ("
                        << in_NumLogDomains << ") is different from the number of logical domains created during "
                        << "the first call to hStreams_app_init* functions (" << globals::app_init_log_doms_IDs.size()
                        << ")";
                HSTR_RETURN(HSTR_RESULT_INCONSISTENT_ARGS);
            } else {
                HSTR_DEBUG1(HSTR_INFO_TYPE_TRACE)
                        << "Subsequent call to an App API-level initialization function, "
                        "will skip creating logical domains and reuse the ones created during the first call.";
            }
        }

        // To wrap logical stream IDs properly, we need a total up front
        // This calculation serves a dual purpose. First, it checks that the user doesn't request
        // zero streams in _all_ the logical domains. Secondly, the variable tot_places will be
        // later used in a call to hStreams_StreamCreate
        const uint32_t tot_places = std::accumulate(in_pStreamsPerDomain, in_pStreamsPerDomain + in_NumLogDomains, 0);
        if (!tot_places) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "Initializing hStreams with 0 streams in all logical domains is prohibited.";

            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }

        // Init() - enumerates, creates processes
        // Call hStreams_Init now and hStreams_Fini on return if needed
        HSTRInitializer hstr_init;
        CHECK_HSTR_RESULT(
            hstr_init.getInitResult());

        HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
                << "Completed initialization, prior to configuring domains and streams.";

        uint32_t       num_phys_domains, num_active_domains;
        bool           dont_care_homog; // we can deal with lack of homogeneity
        // Not possible to get anything other than SUCCESS
        hStreams_GetNumPhysDomains(&num_phys_domains, &num_active_domains,
                                   &dont_care_homog);

        // Spanning all logical domains
        HSTR_LOG_STR        log_str     = 0; // spans all logical domains
        HSTR_LOG_STR        log_str_base = globals::app_init_next_log_str_ID; // spans all logical domains
        HSTR_LOG_DOM        log_dom_idx = 0; // spans all logical domains

        // For physical domains
        uint32_t            phys_domains_limit;
        uint32_t            phys_domain, active_phys_domain;
        int32_t             use_num_threads;
        HSTR_RESULT         ret_val;

        // For GetPhysDomainDetails
        uint32_t            num_threads;
        HSTR_ISA_TYPE       dont_care_ISA;
        uint32_t            dont_care_CoreMaxMHz;
        HSTR_CPU_MASK       max_cpu_mask, avoid_cpu_mask, avail_cpu_mask; // phys domain masks
        uint64_t            phys_mem_types; // not yet used
        uint64_t            phys_bytes_per_mem_type[HSTR_MEM_TYPE_SIZE]; // not yet used

        // For logical domains
        HSTR_LOG_DOM        log_domain_ID;
        HSTR_OVERLAP_TYPE   dont_care_overlap;
        int32_t             d_lower, d_upper; // mask management
        uint32_t            dsize;
        HSTR_LOG_DOM        dom;
        HSTR_CPU_MASK       dom_cpu_mask;

        // For places
        uint32_t            psize, place;
        int32_t             p_lower, p_upper, p_lower_keep; // mask management
        HSTR_CPU_MASK       place_cpu_mask;

        //For logical streams
        HSTR_LOG_STR        lstr;

        // Compute limits on physical domains
        phys_domains_limit = hStreams_GetOptions_phys_domains_limit();
        if (phys_domains_limit > num_active_domains) {
            phys_domains_limit = num_active_domains;
        }
        if (phys_domains_limit > in_NumLogDomains) {
            phys_domains_limit =   in_NumLogDomains;
        }

        // There may be more logical than physical domains, and they may be
        //  mapped unevenly, with more logical domains in physical domains up
        //  to last_phys_upper_domain, and one fewer logical domains after that
        HSTR_LOG_DOM  num_log_domains_upper =
            (in_NumLogDomains + phys_domains_limit - 1) / phys_domains_limit;
        HSTR_LOG_DOM  num_log_domains_lower =
            (in_NumLogDomains) / phys_domains_limit;
        HSTR_PHYS_DOM last_phys_upper_domain = (in_NumLogDomains % phys_domains_limit) - 1;
        HSTR_LOG_DOM  num_log_domains = num_log_domains_upper;

        for (phys_domain = 0, active_phys_domain = 0; active_phys_domain < (HSTR_PHYS_DOM)phys_domains_limit;
                phys_domain++) {

            // Find and skip inactive physical domains
            // Get # threads and cpu masks for this physical domain, which may be unique
            ret_val = hStreams_GetPhysDomainDetails(phys_domain, &num_threads,
                                                    &dont_care_ISA, &dont_care_CoreMaxMHz,
                                                    max_cpu_mask, avoid_cpu_mask,
                                                    &phys_mem_types, phys_bytes_per_mem_type);
            if (ret_val != HSTR_RESULT_SUCCESS) {
                continue;    // Skip, since can only fail here if that domain is inactive
            }

            // Handle case where in_NumLogDomains is not an even multiple of the
            //  number of active physical domains
            if (active_phys_domain > (uint32_t)last_phys_upper_domain) {
                num_log_domains = num_log_domains_lower;
            }

            // Derive CPU masks for each phys domain
            //  reduce num_threads to what is available on this physical domain
            HSTR_CPU_MASK_XOR(avail_cpu_mask, max_cpu_mask, avoid_cpu_mask);
            use_num_threads = HSTR_CPU_MASK_COUNT(avail_cpu_mask);
            HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
                    << "Due to 'avoid' CPU mask, " << use_num_threads
                    << " will be used out of physical domain's " << num_threads << " threads.";

            // Compute size of domain; round down and adjust for remainder
            dsize = use_num_threads / num_log_domains;

            // Prep creation of per-domain masks
            d_lower = 0;
            while (!HSTR_CPU_MASK_ISSET(d_lower, avail_cpu_mask)) {
                d_lower++;
            }

            // Iterate thry log domains within physical domain; log_dom_idx spans all phys domains
            for (dom = 0; dom < num_log_domains; dom++, log_dom_idx++) {
                p_lower = d_lower;
                if (is_first_app_initialization) {
                    HSTR_DEBUG2(HSTR_INFO_TYPE_TRACE) << "Creating a new logical domain.";

                    // Add the remainder for the last domain
                    if (dom == num_log_domains - 1) {
                        dsize += use_num_threads % num_log_domains;
                    }

                    // Form mask for this domain
                    HSTR_CPU_MASK_ZERO(dom_cpu_mask);
                    d_upper = d_lower + dsize;
                    for (; d_lower < d_upper; d_lower++) {
                        HSTR_CPU_MASK_SET(d_lower, dom_cpu_mask);
                    }
                    // Create logical domain
                    ret_val = hStreams_AddLogDomain(phys_domain, dom_cpu_mask,
                                                    &log_domain_ID, &dont_care_overlap);
                    if (ret_val != HSTR_RESULT_SUCCESS) {
                        hstr_init.pleaseFini();
                        HSTR_RETURN(ret_val);
                    }
                    globals::app_init_log_doms_IDs.push_back(log_domain_ID);

                    HSTR_DEBUG3(HSTR_INFO_TYPE_TRACE) << "Created logical domain (ID=" << log_domain_ID << ") out of "
                                                      << num_log_domains << "of width " << dsize << " in physical domain " << phys_domain << " out of "
                                                      << phys_domains_limit << " with CPU mask from threads " << d_lower - dsize << " to " << d_upper - 1;
                } else { // is_first_app_initialization
                    log_domain_ID = globals::app_init_log_doms_IDs.at(log_dom_idx);
                    HSTR_DEBUG2(HSTR_INFO_TYPE_TRACE) << "Reusing logical domain (ID=" << log_domain_ID
                                                      << ") for creation of streams.";
                    HSTR_PHYS_DOM ret_physdom;
                    HSTR_CPU_MASK ret_cpu_mask;
                    ret_val = hStreams_GetLogDomainDetails(log_domain_ID, &ret_physdom, ret_cpu_mask);

                    dsize = HSTR_CPU_MASK_COUNT(ret_cpu_mask);

                    // paranoid-check ret_physdom and ret_cpu_mask ?
                    if (ret_val != HSTR_RESULT_SUCCESS) {
                        hstr_init.pleaseFini();
                        HSTR_RETURN(ret_val);
                    }
                } // is_first_app_initialization


                // Error checking
                if (in_pStreamsPerDomain[log_dom_idx] > dsize) {
                    HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                            << "Cannot init domain " << log_dom_idx << "with more streams ("
                            << in_pStreamsPerDomain[log_dom_idx]
                            << ") than threads available in that domain (" << dsize << ")";


                    hstr_init.pleaseFini();
                    HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
                }

                if (in_pStreamsPerDomain[log_dom_idx] == 0) {
                    continue;
                }

                // Compute size of place; round down and adjust for remainder
                psize = dsize / in_pStreamsPerDomain[log_dom_idx];
                uint32_t psize_remainder = dsize % in_pStreamsPerDomain[log_dom_idx];
                // Prep creation of per-place masks
                // Already set p_lower to old value of d_lower, no need to advance here

                // Iterate thru places per this logical domain
                for (place = 0; place < in_pStreamsPerDomain[log_dom_idx]; place++) {

                    HSTR_CPU_MASK_ZERO(place_cpu_mask);
                    p_upper = p_lower + psize;
                    p_lower_keep = p_lower;
                    // Add the remainder for the first psize_remainder places
                    if (place < psize_remainder) {
                        p_upper += 1;
                    }

                    for (; p_lower < p_upper; p_lower++) {
                        HSTR_CPU_MASK_SET(p_lower, place_cpu_mask);
                    }

                    // Iterate thru logical streams per place; log_str spans all phys domains
                    for (lstr = 0; lstr < (int32_t)in_LogStreamOversubscription; lstr++, log_str++) {
                        HSTR_LOG(HSTR_INFO_TYPE_MISC)
                                << "Creating logical stream " << log_str_base + lstr *tot_places + place
                                << ", partition " << place << ", on logical domain " << log_domain_ID
                                << ", reserving CPU mask from threads " << p_lower_keep << " to " << p_upper - 1;

                        // Create one or more logical stream per physical stream (place)
                        // Want affinity to be like KMP_AFFINITY = scatter
                        //  So logical streams go across places first, vs. within places
                        //  Ex: 2 phys dom, 3,2 log dom, 3,2;2,1 places/dom
                        //               0/10,1/11,2/12, 3/13,4/14 5/15,6/16  7/17,8/18 9/19
                        // places/domain 3               2         2          2         1
                        // log_str_base  0               3         5          7         9
                        //  The math works only because logical streams per place are uniform
                        ret_val = hStreams_StreamCreate(log_str_base + lstr * tot_places + place,
                                                        log_domain_ID, place_cpu_mask);
                        globals::app_init_next_log_str_ID++;
                        if (ret_val != HSTR_RESULT_SUCCESS) {
                            hstr_init.pleaseFini();
                            HSTR_RETURN(ret_val);
                        }
                    } // logical streams per place
                }  // places per logical domain
                log_str_base += in_pStreamsPerDomain[log_dom_idx];
            } // logical domains per physical domain
            active_phys_domain++;
        } // physical domains
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_init , 1)(
        uint32_t     in_StreamsPerDomain,
        uint32_t     in_LogStreamOversubscription)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_StreamsPerDomain);
        HSTR_TRACE_FUN_ARG(in_LogStreamOversubscription);

        // Init() - enumerates, creates processes
        // Set to use all available domains
        // Set to use a uniform number of places per domain
        // Call _init_domains, at least for now

        // Call hStreams_Init now and hStreams_Fini on return if needed
        HSTRInitializer hstr_init;

        //  !in_LogStreamOversubscription checked in app_init_domains
        if (!in_StreamsPerDomain) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "Intializing hStreams with 0 streams per domain is prohibited. "
                    << "If you wish to have zero streams in some (but not all) domains, "
                    << "please use hStreams_app_init_domains.";

            hstr_init.pleaseFini();
            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }

        HSTR_LOG_DOM num_phys_log_domains; // 1 logical domain on each physical domain
        hStreams_COIWrapper::COIEngineGetCount(HSTR_ISA_MIC, &num_phys_log_domains);

        // Check against INIT_MAX_DOMAINS
#define INIT_MAX_DOMAINS 128 // FIXME: ridiculously huge; can change to use STL
        if (num_phys_log_domains > INIT_MAX_DOMAINS) {
            HSTR_ERROR(HSTR_INFO_TYPE_TRACE)
                    << "Requested to have more than INIT_MAX_DOMAINS: "
                    << INIT_MAX_DOMAINS << ". Need to rebuild with a larger value of INIT_MAX_DOMAINS.";

            // unless _hStreams_FatalError is overridden, we should never get here
            hstr_init.pleaseFini();
            HSTR_RETURN(HSTR_RESULT_OUT_OF_RANGE);
        }

        uint32_t places_per_domain[INIT_MAX_DOMAINS];
        // Fill placesPerDomain evenly across
        for (HSTR_LOG_DOM i = 0; i < num_phys_log_domains; i++) {
            places_per_domain[i] = in_StreamsPerDomain;
        }

        HSTR_RESULT hres = hStreams_app_init_domains(num_phys_log_domains,
                           places_per_domain,
                           in_LogStreamOversubscription);
        if (hres != HSTR_RESULT_SUCCESS) {
            hstr_init.pleaseFini();
        }

        return hres;
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_fini , 1)()
    {
        HSTR_TRACE_FUN_ENTER();

        CHECK_HSTR_RESULT(
            hStreams_Fini());
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_create_buf , 1)(void *in_BufAddr,
            const uint64_t in_NumBytes)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_BufAddr);
        HSTR_TRACE_FUN_ARG(in_NumBytes);

        CHECK_HSTR_RESULT(
            hStreams_Alloc1D(
                in_BufAddr,   // address in source domain
                in_NumBytes   // size
            ));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_xfer_memory , 1)(
        void               *in_pReadAddr,
        void               *in_pWriteAddr,
        uint64_t            in_NumBytes,
        HSTR_LOG_STR        in_LogStreamID,
        HSTR_XFER_DIRECTION in_XferDirection,
        HSTR_EVENT         *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_pReadAddr);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_NumBytes);
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_XferDirection);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueData1D(
                in_LogStreamID,    // logical stream
                in_pWriteAddr,     // address in dest domain
                in_pReadAddr,      // address in source domain
                in_NumBytes,       // size
                in_XferDirection,  // xfer direction
                out_pEvent         // completion event
            ));

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }
#endif

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_xfer_memory , 2)(
        HSTR_LOG_STR        in_LogStreamID,
        void               *in_pWriteAddr,
        void               *in_pReadAddr,
        uint64_t            in_NumBytes,
        HSTR_XFER_DIRECTION in_XferDirection,
        HSTR_EVENT         *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_pReadAddr);
        HSTR_TRACE_FUN_ARG(in_NumBytes);
        HSTR_TRACE_FUN_ARG(in_XferDirection);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueData1D(
                in_LogStreamID,    // logical stream
                in_pWriteAddr,     // address in dest domain
                in_pReadAddr,      // address in dest domain
                in_NumBytes,       // size
                in_XferDirection,  // xfer direction
                out_pEvent         // completion event
            ));

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_invoke , 1)(
        HSTR_LOG_STR   in_LogStreamID,
        const char    *in_pFuncName,
        uint32_t       in_NumScalarArgs,
        uint32_t       in_NumHeapArgs,
        uint64_t      *in_pArgs,
        void          *out_pReturnValue,
        uint32_t       in_ReturnValueSize,
        HSTR_EVENT    *out_pEvent
    )
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG_STR(in_pFuncName);
        HSTR_TRACE_FUN_ARG(in_NumScalarArgs);
        HSTR_TRACE_FUN_ARG(in_NumHeapArgs);
        HSTR_TRACE_FUN_ARG(in_pArgs);
        HSTR_TRACE_FUN_ARG(out_pReturnValue);
        HSTR_TRACE_FUN_ARG(in_ReturnValueSize);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                in_pFuncName,
                in_NumScalarArgs,
                in_NumHeapArgs,
                in_pArgs,
                out_pEvent,
                out_pReturnValue,
                in_ReturnValueSize));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }
#endif

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_invoke , 2)(
        HSTR_LOG_STR   in_LogStreamID,
        const char    *in_pFuncName,
        uint32_t       in_NumScalarArgs,
        uint32_t       in_NumHeapArgs,
        uint64_t      *in_pArgs,
        HSTR_EVENT    *out_pEvent,
        void          *out_pReturnValue,
        uint16_t       in_ReturnValueSize
    )
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG_STR(in_pFuncName);
        HSTR_TRACE_FUN_ARG(in_NumScalarArgs);
        HSTR_TRACE_FUN_ARG(in_NumHeapArgs);
        HSTR_TRACE_FUN_ARG(in_pArgs);
        HSTR_TRACE_FUN_ARG(out_pEvent);
        HSTR_TRACE_FUN_ARG(out_pReturnValue);
        HSTR_TRACE_FUN_ARG(in_ReturnValueSize);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                in_pFuncName,
                in_NumScalarArgs,
                in_NumHeapArgs,
                in_pArgs,
                out_pEvent,
                out_pReturnValue,
                in_ReturnValueSize));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_stream_sync , 1)(HSTR_LOG_STR in_LogStreamID)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);

        CHECK_HSTR_RESULT(
            hStreams_StreamSynchronize(in_LogStreamID));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_thread_sync , 1)()
    {
        HSTR_TRACE_FUN_ENTER();

        CHECK_HSTR_RESULT(hStreams_IsInitialized());
        CHECK_HSTR_RESULT(
            hStreams_ThreadSynchronize());
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_event_wait , 1)(
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_NumEvents);
        HSTR_TRACE_FUN_ARG(in_pEvents);

        HSTR_OPTIONS options;
        hStreams_GetCurrentOptions(&options, sizeof(HSTR_OPTIONS));

        CHECK_HSTR_RESULT(
            hStreams_EventWait(
                in_NumEvents,
                in_pEvents, // event to wait on
                true,       // Assume WaitForAll = true
                options.time_out_ms_val,
                NULL, NULL  // Assume don't care about signaled events
            ));
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_event_wait_in_stream , 1)(
        HSTR_LOG_STR       in_LogStreamID,
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents,
        int32_t            in_NumAddresses,
        void             **in_pAddresses,
        HSTR_EVENT         *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_NumEvents);
        HSTR_TRACE_FUN_ARG(in_NumAddresses);
        HSTR_TRACE_FUN_ARG(in_pAddresses);

        return hStreams_EventStreamWait(in_LogStreamID,
                                        in_NumEvents,
                                        in_pEvents,
                                        in_NumAddresses,
                                        in_pAddresses,
                                        out_pEvent);
    }


#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_memset , 1)(void *in_Dest, int in_Val, uint64_t in_NumBytes,
            HSTR_LOG_STR in_LogStreamID, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_Dest);
        HSTR_TRACE_FUN_ARG(in_Val);
        HSTR_TRACE_FUN_ARG(in_NumBytes);
        HSTR_TRACE_FUN_ARG(in_LogStreamID);

        uint64_t args[3];

        // Check in_Dest is not NULL
        if (!in_Dest) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        args[0] = in_NumBytes;
        args[1] = in_Val;
        args[2] = *((uint64_t *)(&in_Dest));

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                "hStreams_memset_sink",
                2,             // scalar args
                1,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }
#endif

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_memset , 2)(HSTR_LOG_STR in_LogStreamID, void *in_pWriteAddr, int in_Val, uint64_t in_NumBytes,
            HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_Val);
        HSTR_TRACE_FUN_ARG(in_NumBytes);

        uint64_t args[3];

        // Check in_pWriteAddr is not NULL
        if (!in_pWriteAddr) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        args[0] = in_NumBytes;
        args[1] = in_Val;
        args[2] = *((uint64_t *)(&in_pWriteAddr));

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                "hStreams_memset_sink",
                2,             // scalar args
                1,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_memcpy , 1)(void *in_Src, void *in_Dest, uint64_t in_NumBytes,
            HSTR_LOG_STR in_LogStreamID, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_Src);
        HSTR_TRACE_FUN_ARG(in_Dest);
        HSTR_TRACE_FUN_ARG(in_NumBytes);
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        uint64_t args[3];

        // Check in_Dest or in_Src is not NULL
        if (!in_Dest || !in_Src) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        args[0] = in_NumBytes;
        args[1] = (uint64_t)(in_Src);
        args[2] = (uint64_t)(in_Dest);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                "hStreams_memcpy_sink",
                1,             // scalar args
                2,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }
#endif

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_memcpy , 2)(HSTR_LOG_STR in_LogStreamID, void *in_pReadAddr, void *in_pWriteAddr, uint64_t in_NumBytes,
            HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(in_LogStreamID);
        HSTR_TRACE_FUN_ARG(in_pReadAddr);
        HSTR_TRACE_FUN_ARG(in_pWriteAddr);
        HSTR_TRACE_FUN_ARG(in_NumBytes);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        uint64_t args[3];

        // Check in_pWriteAddr or in_pReadAddr is not NULL
        if (!in_pWriteAddr || !in_pReadAddr) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        args[0] = in_NumBytes;
        args[1] = (uint64_t)(in_pReadAddr);
        args[2] = (uint64_t)(in_pWriteAddr);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                "hStreams_memcpy_sink",
                1,             // scalar args
                2,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

    static HSTR_RESULT hStreams_app_sgemm_worker(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const float alpha,
        const float *A, const int64_t lda,
        const float *B, const int64_t ldb, const float beta,
        float *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        uint64_t args[14];
        floatToUint64_t uAlpha, uBeta;

        // Check A, B or C is not NULL
        if (!A || !B || !C) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        uAlpha.Set(alpha);
        uBeta.Set(beta);
        args[ 0] = (uint64_t)(Order);
        args[ 1] = (uint64_t)(TransA);
        args[ 2] = (uint64_t)(TransB);
        args[ 3] = (uint64_t)(M);
        args[ 4] = (uint64_t)(N);
        args[ 5] = (uint64_t)(K);
        args[ 6] = (uint64_t)(uAlpha.Get_uint64_t());
        args[ 7] = (uint64_t)(lda);
        args[ 8] = (uint64_t)(ldb);
        args[ 9] = (uint64_t)(uBeta.Get_uint64_t());
        args[10] = (uint64_t)(ldc);
        args[11] = (uint64_t)(A);
        args[12] = (uint64_t)(B);
        args[13] = (uint64_t)(C);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                LogStream,
                "hStreams_sgemm_sink",
                11,            // scalar args
                3,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_sgemm , 1)(
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const float alpha,
        const float *A, const MKL_INT lda,
        const float *B, const MKL_INT ldb, const float beta,
        float *C, const MKL_INT ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_sgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }


    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_sgemm , 2)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const float alpha,
        const float *A, const MKL_INT lda,
        const float *B, const MKL_INT ldb, const float beta,
        float *C, const MKL_INT ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_sgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }
#endif
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_sgemm , 3)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const float alpha,
        const float *A, const int64_t lda,
        const float *B, const int64_t ldb, const float beta,
        float *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_sgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }


    static HSTR_RESULT hStreams_app_dgemm_worker(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const double alpha,
        const double *A, const int64_t lda,
        const double *B, const int64_t ldb, const double beta,
        double *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        doubleToUint64_t uAlpha, uBeta;
        uint64_t args[14];

        // Check A, B or C is not NULL
        if (!A || !B || !C) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        uAlpha.Set(alpha);
        uBeta.Set(beta);

        // Set up scalar args, then heap args
        args[ 0] = (uint64_t)(Order);
        args[ 1] = (uint64_t)(TransA);
        args[ 2] = (uint64_t)(TransB);
        args[ 3] = (uint64_t)(M);
        args[ 4] = (uint64_t)(N);
        args[ 5] = (uint64_t)(K);
        args[ 6] = (uint64_t)(uAlpha.Get_uint64_t());
        args[ 7] = (uint64_t)(lda);
        args[ 8] = (uint64_t)(ldb);
        args[ 9] = (uint64_t)(uBeta.Get_uint64_t());
        args[10] = (uint64_t)(ldc);
        args[11] = (uint64_t)(A);
        args[12] = (uint64_t)(B);
        args[13] = (uint64_t)(C);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                LogStream,
                "hStreams_dgemm_sink",
                11,            // scalar args
                3,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_dgemm , 1)(
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const double alpha,
        const double *A, const MKL_INT lda,
        const double *B, const MKL_INT ldb, const double beta,
        double *C, const MKL_INT ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
    {

        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_dgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_dgemm , 2)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const double alpha,
        const double *A, const MKL_INT lda,
        const double *B, const MKL_INT ldb, const double beta,
        double *C, const MKL_INT ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_dgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }
#endif
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_dgemm , 3)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const double alpha,
        const double *A, const int64_t lda,
        const double *B, const int64_t ldb, const double beta,
        double *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_dgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }

    static HSTR_RESULT hStreams_app_cgemm_worker(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const void *alpha,
        const void *A, const int64_t lda,
        const void *B, const int64_t ldb, const void *beta,
        void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        uint64_t args[14];
        MKL_Complex8ToUint64_t uAlpha, uBeta;

        // Check A, B or C is not NULL
        if (!A || !B || !C) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        uAlpha.Set(*((MKL_Complex8 *)alpha));
        uBeta.Set(*((MKL_Complex8 *)beta));
        args[ 0] = (uint64_t)(Order);
        args[ 1] = (uint64_t)(TransA);
        args[ 2] = (uint64_t)(TransB);
        args[ 3] = (uint64_t)(M);
        args[ 4] = (uint64_t)(N);
        args[ 5] = (uint64_t)(K);
        args[ 6] = (uint64_t)(uAlpha.Get_uint64_t());
        args[ 7] = (uint64_t)(lda);
        args[ 8] = (uint64_t)(ldb);
        args[ 9] = (uint64_t)(uBeta.Get_uint64_t());
        args[10] = (uint64_t)(ldc);
        args[11] = (uint64_t)(A);
        args[12] = (uint64_t)(B);
        args[13] = (uint64_t)(C);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                LogStream,
                "hStreams_cgemm_sink",
                11,            // scalar args
                3,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }
#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_cgemm , 1)(
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const void *alpha,
        const void *A, const MKL_INT lda,
        const void *B, const MKL_INT ldb, const void *beta,
        void *C, const MKL_INT ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_cgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_cgemm , 2)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const void *alpha,
        const void *A, const MKL_INT lda,
        const void *B, const MKL_INT ldb, const void *beta,
        void *C, const MKL_INT ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_cgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }
#endif
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_cgemm , 3)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const void *alpha,
        const void *A, const int64_t lda,
        const void *B, const int64_t ldb, const void *beta,
        void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_cgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }


    static HSTR_RESULT hStreams_app_zgemm_worker(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const void *alpha,
        const void *A, const int64_t lda,
        const void *B, const int64_t ldb, const void *beta,
        void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)

    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        uint64_t args[16];
        MKL_Complex16ToUint64_t uAlpha, uBeta;

        // Check A, B or C is not NULL
        if (!A || !B || !C) {
            HSTR_RETURN(HSTR_RESULT_NULL_PTR);
        }

        // Set up scalar args, then heap args
        uAlpha.Set(*((MKL_Complex16 *)alpha));
        uBeta.Set(*((MKL_Complex16 *)beta));
        args[ 0] = (uint64_t)(Order);
        args[ 1] = (uint64_t)(TransA);
        args[ 2] = (uint64_t)(TransB);
        args[ 3] = (uint64_t)(M);
        args[ 4] = (uint64_t)(N);
        args[ 5] = (uint64_t)(K);
        args[ 6] = (uint64_t)(uAlpha.Get_uint64_t(0));
        args[ 7] = (uint64_t)(uAlpha.Get_uint64_t(1));
        args[ 8] = (uint64_t)(lda);
        args[ 9] = (uint64_t)(ldb);
        args[10] = (uint64_t)(uBeta.Get_uint64_t(0));
        args[11] = (uint64_t)(uBeta.Get_uint64_t(1));
        args[12] = (uint64_t)(ldc);
        args[13] = (uint64_t)(A);
        args[14] = (uint64_t)(B);
        args[15] = (uint64_t)(C);

        CHECK_HSTR_RESULT(
            hStreams_EnqueueCompute(
                LogStream,
                "hStreams_zgemm_sink",
                13,            // scalar args
                3,             // heap args
                args,          // arg array
                out_pEvent,    // event
                NULL,          // return value pointer
                0));           // return value size

        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    }

#ifndef _WIN32
    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_zgemm , 1)(
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const void *alpha,
        const void *A, const MKL_INT lda,
        const void *B, const MKL_INT ldb, const void *beta,
        void *C, const MKL_INT ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_zgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }


    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_zgemm , 2)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const MKL_INT M, const MKL_INT N, const MKL_INT K, const void *alpha,
        const void *A, const MKL_INT lda,
        const void *B, const MKL_INT ldb, const void *beta,
        void *C, const MKL_INT ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_zgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }
#endif

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_app_zgemm , 3)(
        HSTR_LOG_STR LogStream,
        const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
        const int64_t M, const int64_t N, const int64_t K, const void *alpha,
        const void *A, const int64_t lda,
        const void *B, const int64_t ldb, const void *beta,
        void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
    {
        HSTR_TRACE_FUN_ENTER();
        HSTR_TRACE_FUN_ARG(LogStream);
        HSTR_TRACE_FUN_ARG(Order);
        HSTR_TRACE_FUN_ARG(TransA);
        HSTR_TRACE_FUN_ARG(TransB);
        HSTR_TRACE_FUN_ARG(M);
        HSTR_TRACE_FUN_ARG(N);
        HSTR_TRACE_FUN_ARG(K);
        HSTR_TRACE_FUN_ARG(alpha);
        HSTR_TRACE_FUN_ARG(A);
        HSTR_TRACE_FUN_ARG(lda);
        HSTR_TRACE_FUN_ARG(B);
        HSTR_TRACE_FUN_ARG(ldb);
        HSTR_TRACE_FUN_ARG(beta);
        HSTR_TRACE_FUN_ARG(C);
        HSTR_TRACE_FUN_ARG(ldc);
        HSTR_TRACE_FUN_ARG(out_pEvent);

        return hStreams_app_zgemm_worker(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
    }

} // extern "C"
