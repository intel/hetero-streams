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
#include <stdarg.h>
#include <string.h>
#include "hStreams_internal.h"
#include "hStreams_common.h"
#include "hStreams_exceptions.h"
#include "hStreams_internal_vars_common.h"
#include "hStreams_Logger.h"

#if (!defined _WIN32) && (defined __cplusplus)
extern "C" {
#endif

    uint32_t HSTR_SYMBOL_VERSION(hStreams_GetVerbose , 1)()
    {
        return hStreams_GetOptions_verbose();
    }

    HSTR_RESULT HSTR_SYMBOL_VERSION(hStreams_SetVerbose , 1)(int target_verbosity)
    {
        HSTR_OPTIONS currentOptions;

        hStreams_GetCurrentOptions(&currentOptions, sizeof(currentOptions));
        currentOptions.verbose = target_verbosity;
        hStreams_SetOptions(&currentOptions);
        return HSTR_RESULT_SUCCESS;
    }

    HSTR_RESULT HSTR_SYMBOL_VERSION(hStreams_GetCurrentOptions , 1)(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize)
    {
        try {
            if (pCurrentOptions && buffSize >= sizeof(HSTR_OPTIONS)) {
                hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(global_options_lock,
                        hStreams_RW_Lock::HSTR_RW_LOCK_READ);

                *pCurrentOptions = global_options;
                return HSTR_RESULT_SUCCESS;
            }
            return HSTR_RESULT_OUT_OF_RANGE;
        } catch (...) {
            return hStreams_handle_exception();
        }
    }

    // The DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION() macro defines a thread-safe function
    // for getting one member of the HSTR_OPTIONS struct.  These functions are declared in
    // hStreams_internal.h
#define DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(MEMBER_NAME)                                       \
    extern HSTR_TYPEOF(((HSTR_OPTIONS*)0)->MEMBER_NAME) hStreams_GetOptions_ ## MEMBER_NAME (void) \
    {                                                                                              \
        try {                                                                                      \
            hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(global_options_lock,      \
                    hStreams_RW_Lock::HSTR_RW_LOCK_READ);                                          \
            return global_options . MEMBER_NAME;                                         \
        } catch (...) {                                                                            \
            hStreams_handle_exception();                                                           \
            /* Unlocked return fallback */                                                         \
            return global_options . MEMBER_NAME;                                         \
        }                                                                                          \
    }

    // Define each of the thread safe functions for getting the current value of
    // the members of the hstreams options structure:
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(verbose)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(dep_policy)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(phys_domains_limit)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(openmp_policy)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(time_out_ms_val)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(_hStreams_FatalError)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(kmp_affinity)
    DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(_hStreams_EmitMessage)

    /**********************************************************************************

    Here, we define the static implementation of the externally defined, and versioned
    function hSteams_SetOptions().  The second argument indicates what version the
    code should implement.

    Note the references to the worker function in the below external definitions.

    ***********************************************************************************/
    static HSTR_RESULT hStreams_SetOptions_worker(const HSTR_OPTIONS *in_options, int hstreams_library_version)
    {
        try {
            if (hstreams_library_version != HSTR_SECOND_PREVIEW_LIBRARY_VERSION) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                        << "SetOptions_worker was called with an unsupported hStreams library version: "
                        << hstreams_library_version;

                // unless _hStreams_FatalError is overriden, we should never get here
                return HSTR_RESULT_INTERNAL_ERROR;
            }
            if (in_options == NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Options can't be NULL.";

                return HSTR_RESULT_NULL_PTR;
            }
            if (in_options->_hStreams_EmitMessage != NULL) {
                HSTR_WARN(HSTR_INFO_TYPE_MISC) << "_hStreams_EmitMessage functionality is deprecated and removed.";
            }
            if (in_options->_hStreams_FatalError == NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "_hStreams_FatalError can't be NULL.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            if (in_options->kmp_affinity < 0 || in_options->kmp_affinity >= HSTR_KMP_AFFINITY_SIZE) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Incorrect value of kmp_affinity.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            if (in_options->dep_policy < 0 || in_options->dep_policy >= HSTR_DEP_POLICY_SIZE) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Incorrect value of dep_policy.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            if (in_options->openmp_policy < 0 || in_options->openmp_policy >= HSTR_OPENMP_POLICY_SIZE) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Incorrect value of openmp_policy.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            if (in_options->time_out_ms_val <= 0 && in_options->time_out_ms_val != HSTR_TIME_INFINITE) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Incorrect value of time_out_ms_val.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            if (in_options->libNameCnt == 0 && in_options->libNames != NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNames must be NULL, if libNameCnt is zero.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
#ifdef HSTR_SOURCE
            if (in_options->libNameCnt != 0 && in_options->libNames == NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNames can't be NULL, if libNameCnt isn't zero.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            for (int i = 0; i < in_options->libNameCnt; i++) {
                if (in_options->libNames[i] == NULL) {
                    HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNames can't contain NULL entries.";

                    return HSTR_RESULT_INCONSISTENT_ARGS;
                }
            }
#endif
            if (in_options->libNameCntHost == 0 && in_options->libNamesHost != NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNamesHost must be NULL, if libNameCntHost is zero.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
#ifdef HSTR_SOURCE
            if (in_options->libNameCntHost != 0 && in_options->libNamesHost == NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNamesHost can't be NULL, if libNameCntHost isn't zero.";

                return HSTR_RESULT_INCONSISTENT_ARGS;
            }
            for (int i = 0; i < in_options->libNameCntHost; i++) {
                if (in_options->libNamesHost[i] == NULL) {
                    HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "libNamesHost can't contain NULL entries.";

                    return HSTR_RESULT_INCONSISTENT_ARGS;
                }
            }
#endif
            hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(global_options_lock, hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
            global_options = *in_options;
            return HSTR_RESULT_SUCCESS;
        } catch (...) {
            return hStreams_handle_exception();
        }
    }

    /****************************************************************************

    This is only a wrapper function referencing the implementation - hStreams_SetOptions_worker

    *****************************************************************************/

    HSTR_RESULT HSTR_SYMBOL_VERSION(hStreams_SetOptions , 1)(const HSTR_OPTIONS *in_options)
    {
        try {
            return hStreams_SetOptions_worker(in_options, HSTR_SECOND_PREVIEW_LIBRARY_VERSION);
        } catch (...) {
            return hStreams_handle_exception();
        }
    }

    void hStreams_ResetDefaultOptions(void)
    {
        try {
            static const HSTR_OPTIONS hstr_options_tmp = HSTR_OPTIONS_INITIAL_VALUES;
            hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(global_options_lock, hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
            memcpy(&global_options, &hstr_options_tmp, sizeof(HSTR_OPTIONS));
        } catch (...) {
            // ignore the error code
            hStreams_handle_exception();
        }
    }

    // Fill out the hStreamsResultNames const array:
    static const char *hStreamsResultNames[] = {
        "HSTR_RESULT_SUCCESS",
        "HSTR_RESULT_REMOTE_ERROR",
        "HSTR_RESULT_NOT_INITIALIZED",
        "HSTR_RESULT_NOT_FOUND",
        "HSTR_RESULT_ALREADY_FOUND",
        "HSTR_RESULT_OUT_OF_RANGE",
        "HSTR_RESULT_DOMAIN_OUT_OF_RANGE",
        "HSTR_RESULT_CPU_MASK_OUT_OF_RANGE",
        "HSTR_RESULT_OUT_OF_MEMORY",
        "HSTR_RESULT_INVALID_STREAM_TYPE",
        "HSTR_RESULT_OVERLAPPING_RESOURCES",
        "HSTR_RESULT_DEVICE_NOT_INITIALIZED",
        "HSTR_RESULT_BAD_NAME",
        "HSTR_RESULT_TOO_MANY_ARGS",
        "HSTR_RESULT_TIME_OUT_REACHED",
        "HSTR_RESULT_EVENT_CANCELED",
        "HSTR_RESULT_INCONSISTENT_ARGS",
        "HSTR_RESULT_BUFF_TOO_SMALL",
        "HSTR_RESULT_MEMORY_OPERAND_INCONSISTENT",
        "HSTR_RESULT_NULL_PTR",
        "HSTR_RESULT_INTERNAL_ERROR",
        "HSTR_RESULT_RESOURCE_EXHAUSTED",
        "HSTR_RESULT_NOT_IMPLEMENTED",
        "HSTR_RESULT_NOT_PERMITTED"
    };

    HSTR_STATIC_ASSERT(sizeof(hStreamsResultNames) == HSTR_RESULT_SIZE *sizeof(const char *), invalid_number_of_hStreamsResultNames_entries);

    /////////////////////////////////////////////////////////
    /// Get HSTR_RESULT name
    /////////////////////////////////////////////////////////

    const char *HSTR_SYMBOL_VERSION(hStreams_ResultGetName , 1)(
        HSTR_RESULT n)
    {
        if (n >= 0 && n < HSTR_RESULT_SIZE) {
            return hStreamsResultNames[n];
        } else {
            return "[HSTR_RESULT value out of range]";
        }
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_GetVersionStringLen , 1)(uint32_t *out_pVersionStringLen)
    {
        if (NULL == out_pVersionStringLen) {
            return HSTR_RESULT_NULL_PTR;
        }
        *out_pVersionStringLen = (uint32_t)strlen(version_string) + 1; // include null termination
        return HSTR_RESULT_SUCCESS;
    }

    HSTR_RESULT
    HSTR_SYMBOL_VERSION(hStreams_Version , 1)(char *buff, uint32_t buffLength)
    {
        if (NULL == buff) {
            return HSTR_RESULT_NULL_PTR;
        }

        if (buffLength < (uint32_t)strlen(version_string) + 1) {
            return HSTR_RESULT_BUFF_TOO_SMALL;
        }
        memcpy(buff, version_string, strlen(version_string) + 1);
        return HSTR_RESULT_SUCCESS;
    }

#if (!defined _WIN32) && (defined __cplusplus)
}
#endif
