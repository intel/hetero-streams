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

#include "hStreams_app_api_workers_source.h"
#include "hStreams_core_api_workers_source.h"
#include "hStreams_exceptions.h"
#include "hStreams_helpers_source.h"
#include "hStreams_Logger.h"

#include "hStreams_internal_vars_source.h"

#include <vector>
#include <numeric>

void
detail::app_init_domains_in_version_impl_throw(
    uint32_t     in_NumLogDomains,
    uint32_t    *in_pStreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_NumLogDomains);
    HSTR_TRACE_FUN_ARG(in_pStreamsPerDomain);
    HSTR_TRACE_FUN_ARG(in_LogStreamOversubscription);
    HSTR_TRACE_FUN_ARG_STR(in_InterfaceVersion);

    // Init() - enumerates, creates processes
    // Call hStreams_Init now and hStreams_Fini upon destruction unless its
    // member function dontFini is called.
    HSTRInitializer hstr_init(in_InterfaceVersion);
    if (hstr_init.getInitResult() != HSTR_RESULT_SUCCESS) {
        throw HSTR_EXCEPTION_MACRO(hstr_init.getInitResult(), StringBuilder()
                                   << "Could not initialize the library"
                                  );
    }

    if (in_pStreamsPerDomain == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_pStreamsPerDomain cannot be NULL"
                                  );
    }
    if (!in_NumLogDomains) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "App FUN-level initialization of the Hetero Streams Library with 0 "
                                   <<  "logical domains is prohibited"
                                  );
    }
    if (!in_LogStreamOversubscription) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "App FUN-level initialization of the Hetero Streams Library with 0 "
                                   <<  "logical streams per physical stream is prohibited"
                                  );
    }

    bool is_first_app_initialization;
    // verify if this is the first time the App-FUN level initialization functions are being called
    if (globals::app_init_log_doms_IDs.empty()) {
        is_first_app_initialization = true;
        HSTR_DEBUG1(HSTR_INFO_TYPE_TRACE)
                << "First call to and App FUN-level initialization function.";
    } else {
        is_first_app_initialization = false;
        if (in_NumLogDomains != globals::app_init_log_doms_IDs.size()) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INCONSISTENT_ARGS, StringBuilder()
                                       << "The requested number of logical domains to be used ("
                                       << in_NumLogDomains
                                       << ") is different from the number of logical domains created during "
                                       << "the first call to hStreams_app_init* functions ("
                                       << globals::app_init_log_doms_IDs.size()
                                       << ")"
                                      );
        } else {
            HSTR_DEBUG1(HSTR_INFO_TYPE_TRACE)
                    << "Subsequent call to an App FUN-level initialization function, "
                    << "will skip creating logical domains and reuse the ones created during the first call.";
        }
    }

    // To wrap logical stream IDs properly, we need a total up front
    // This calculation serves a dual purpose. First, it checks that the user doesn't request
    // zero streams in _all_ the logical domains. Secondly, the variable tot_places will be
    // later used in a call to hStreams_StreamCreate
    const uint32_t tot_places = std::accumulate(in_pStreamsPerDomain, in_pStreamsPerDomain + in_NumLogDomains, 0);
    if (!tot_places) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Initializing hStreams with 0 streams in all logical domains is prohibited."
                                  );
    }

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
                    throw HSTR_EXCEPTION_MACRO(ret_val, StringBuilder()
                                               << "Error while attempting to create a logical domain "
                                               << "in physical domain (ID="
                                               << phys_domain
                                               << ")"
                                              );
                }
                globals::app_init_log_doms_IDs.push_back(log_domain_ID);

                HSTR_DEBUG3(HSTR_INFO_TYPE_TRACE)
                        << "Created logical domain (ID="
                        << log_domain_ID
                        << ") out of "
                        << num_log_domains
                        << "of width "
                        << dsize
                        << " in physical domain "
                        << phys_domain
                        << " out of "
                        << phys_domains_limit
                        << " with CPU mask from threads "
                        << d_lower - dsize
                        << " to "
                        << d_upper - 1;
            } else { // is_first_app_initialization
                log_domain_ID = globals::app_init_log_doms_IDs.at(log_dom_idx);
                HSTR_DEBUG2(HSTR_INFO_TYPE_TRACE)
                        << "Reusing logical domain (ID="
                        << log_domain_ID
                        << ") for creation of streams.";
                HSTR_PHYS_DOM ret_physdom;
                HSTR_CPU_MASK ret_cpu_mask;
                ret_val = hStreams_GetLogDomainDetails(log_domain_ID, &ret_physdom, ret_cpu_mask);

                dsize = HSTR_CPU_MASK_COUNT(ret_cpu_mask);

                // paranoid-check ret_physdom and ret_cpu_mask ?
                if (ret_val != HSTR_RESULT_SUCCESS) {
                    throw HSTR_EXCEPTION_MACRO(ret_val, StringBuilder()
                                               << "Internal error while getting logical domain details."
                                              );
                }
            } // is_first_app_initialization


            // Error checking
            if (in_pStreamsPerDomain[log_dom_idx] > dsize) {
                throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                           << "Cannot init domain " << log_dom_idx << "with more streams ("
                                           << in_pStreamsPerDomain[log_dom_idx]
                                           << ") than threads available in that domain (" << dsize << ")"
                                          );
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
                    // Create one or more logical stream per physical stream (place)
                    // Want affinity to be like KMP_AFFINITY = scatter
                    //  So logical streams go across places first, vs. within places
                    //  Ex: 2 phys dom, 3,2 log dom, 3,2;2,1 places/dom
                    //               0/10,1/11,2/12, 3/13,4/14 5/15,6/16  7/17,8/18 9/19
                    // places/domain 3               2         2          2         1
                    // log_str_base  0               3         5          7         9
                    //  The math works only because logical streams per place are uniform
                    HSTR_LOG_STR log_stream_id = log_str_base + lstr * tot_places + place;
                    HSTR_LOG(HSTR_INFO_TYPE_MISC)
                            << "Creating logical stream "
                            << log_stream_id
                            << ", partition "
                            << place
                            << ", on logical domain "
                            << log_domain_ID
                            << ", reserving CPU mask from threads "
                            << p_lower_keep
                            << " to "
                            << p_upper - 1;

                    ret_val = hStreams_StreamCreate(log_stream_id,
                                                    log_domain_ID, place_cpu_mask);
                    globals::app_init_next_log_str_ID++;
                    if (ret_val != HSTR_RESULT_SUCCESS) {
                        throw HSTR_EXCEPTION_MACRO(ret_val, StringBuilder()
                                                   << "Error while creating the logical stream"
                                                   << " (ID="
                                                   << log_stream_id
                                                   << ") on logical domain (ID="
                                                   << log_domain_ID
                                                   << ")"
                                                  );
                    }
                } // logical streams per place
            }  // places per logical domain
            log_str_base += in_pStreamsPerDomain[log_dom_idx];
        } // logical domains per physical domain
        active_phys_domain++;
    } // physical domains
    hstr_init.dontFini();
} // detail::app_init_domains_in_version_impl_throw

void
detail::app_init_in_version_impl_throw(
    uint32_t     in_StreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_StreamsPerDomain);
    HSTR_TRACE_FUN_ARG(in_LogStreamOversubscription);
    HSTR_TRACE_FUN_ARG_STR(in_InterfaceVersion);

    // Init() - enumerates, creates processes
    // Set to use all available domains
    // Set to use a uniform number of places per domain
    // Call _init_domains, at least for now

    // Call hStreams_Init now and hStreams_Fini on return if needed
    HSTRInitializer hstr_init(in_InterfaceVersion);

    //  !in_LogStreamOversubscription checked in app_init_domains
    if (!in_StreamsPerDomain) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_RANGE, StringBuilder()
                                   << "Intializing hStreams with 0 streams per domain is prohibited. "
                                   << "If you wish to have zero streams in some (but not all) domains, "
                                   << "please use hStreams_app_init_domains."
                                  );
    }

    HSTR_LOG_DOM num_phys_log_domains; // 1 logical domain on each physical domain
    hStreams_COIWrapper::COIEngineGetCount(HSTR_ISA_MIC, &num_phys_log_domains);
    std::vector<uint32_t> places_per_domain(num_phys_log_domains, in_StreamsPerDomain);

    app_init_domains_in_version_impl_throw(
        num_phys_log_domains,
        places_per_domain.data(),
        in_LogStreamOversubscription,
        in_InterfaceVersion);
    // We've made it this far, so everything's ok
    hstr_init.dontFini();
} // detail::app_init_in_version_impl_throw

void
detail::app_event_wait_impl_throw(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_NumEvents);
    HSTR_TRACE_FUN_ARG(in_pEvents);

    HSTR_OPTIONS options;
    hStreams_GetCurrentOptions(&options, sizeof(HSTR_OPTIONS));

    EventWait_impl_throw(in_NumEvents, in_pEvents, true,
                         options.time_out_ms_val, NULL, NULL);
} // detail::app_event_wait_impl_throw

void
detail::app_memset_impl_throw(
    HSTR_LOG_STR   in_LogStreamID,
    void          *in_pWriteAddr,
    int            in_Val,
    uint64_t       in_NumBytes,
    HSTR_EVENT    *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_pWriteAddr);
    HSTR_TRACE_FUN_ARG(in_Val);
    HSTR_TRACE_FUN_ARG(in_NumBytes);

    if (in_pWriteAddr == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "in_pWriteAddr cannot be NULL"
                                  );
    }

    uint64_t args[3];
    args[0] = in_NumBytes;
    args[1] = in_Val;
    args[2] = *((uint64_t *)(&in_pWriteAddr));

    EnqueueCompute_impl_throw(
        in_LogStreamID,
        "hStreams_memset_sink",
        2,             // scalar args
        1,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_memset_impl_throw

void
detail::app_memcpy_impl_throw(
    HSTR_LOG_STR     in_LogStreamID,
    void            *in_pReadAddr,
    void            *in_pWriteAddr,
    uint64_t         in_NumBytes,
    HSTR_EVENT      *out_pEvent)
{
    HSTR_TRACE_FUN_ENTER();
    HSTR_TRACE_FUN_ARG(in_LogStreamID);
    HSTR_TRACE_FUN_ARG(in_pReadAddr);
    HSTR_TRACE_FUN_ARG(in_pWriteAddr);
    HSTR_TRACE_FUN_ARG(in_NumBytes);
    HSTR_TRACE_FUN_ARG(out_pEvent);

    // Check in_pWriteAddr or in_pReadAddr is not NULL
    if (in_pWriteAddr == NULL || in_pReadAddr == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "Neither in_pWriteAddr nor in_pReturnValue can be NULL"
                                  );
    }

    uint64_t args[3];
    args[0] = in_NumBytes;
    args[1] = (uint64_t)(in_pReadAddr);
    args[2] = (uint64_t)(in_pWriteAddr);

    EnqueueCompute_impl_throw(
        in_LogStreamID,
        "hStreams_memcpy_sink",
        1,             // scalar args
        2,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_memcpy_impl_throw

void
detail::app_sgemm_impl_throw(
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

    if (!A || !B || !C) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "None of A,B or C can be NULL"
                                  );
    }

    uint64_t args[14];
    floatToUint64_t uAlpha, uBeta;
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

    EnqueueCompute_impl_throw(
        LogStream,
        "hStreams_sgemm_sink",
        11,            // scalar args
        3,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_sgemm_worker_impl_throw

void
detail::app_dgemm_impl_throw(
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

    if (!A || !B || !C) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "None of A,B or C can be NULL"
                                  );
    }

    doubleToUint64_t uAlpha, uBeta;
    uAlpha.Set(alpha);
    uBeta.Set(beta);
    uint64_t args[14];
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

    EnqueueCompute_impl_throw(
        LogStream,
        "hStreams_dgemm_sink",
        11,            // scalar args
        3,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_dgemm_impl_throw

void
detail::app_cgemm_impl_throw(
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

    if (!A || !B || !C) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "None of A,B or C can be NULL"
                                  );
    }

    MKL_Complex8ToUint64_t uAlpha, uBeta;
    // Set up scalar args, then heap args
    uAlpha.Set(*((MKL_Complex8 *)alpha));
    uBeta.Set(*((MKL_Complex8 *)beta));
    uint64_t args[14];
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

    EnqueueCompute_impl_throw(
        LogStream,
        "hStreams_cgemm_sink",
        11,            // scalar args
        3,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_cgemm_impl_throw

void
detail::app_zgemm_impl_throw(
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

    if (!A || !B || !C) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_NULL_PTR, StringBuilder()
                                   << "None of A,B or C can be NULL"
                                  );
    }

    MKL_Complex16ToUint64_t uAlpha, uBeta;
    // Set up scalar args, then heap args
    uAlpha.Set(*((MKL_Complex16 *)alpha));
    uBeta.Set(*((MKL_Complex16 *)beta));
    uint64_t args[16];
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

    EnqueueCompute_impl_throw(
        LogStream,
        "hStreams_zgemm_sink",
        13,            // scalar args
        3,             // heap args
        args,          // arg array
        out_pEvent,    // event
        NULL,          // return value pointer
        0);            // return value size
} // detail::app_zgemm_impl_throw
