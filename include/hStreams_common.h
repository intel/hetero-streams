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

#ifndef __HSTR_COMMON_H__
#define __HSTR_COMMON_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "hStreams_version.h"
#include "hStreams_types.h"
#include <string.h>
#include <stdio.h>

#ifndef _WIN32
#include <sched.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#endif //DOXYGEN_SHOULD_SKIP_THIS

// /////////////////////////////////////////////////////////////////
// Author: CJ Newburn
// Purpose: Define the API common to both source and sink for hStreams
//

// Defines that pertain to dependence enforcement for waits
#define HSTR_WAIT_CONTROL 0 // Control dependence on all actions
#define HSTR_WAIT_NONE -1   // No control or data dependences

// The maximum size of any user-defined function value
#define HSTR_RETURN_SIZE_LIMIT 64

// The number of arguments that hStreams_EnqueueCompute supports, that are a combination of
// scalars and heap argument for this version of hStreams:
#define HSTR_ARGS_IMPLEMENTED 19

// A special value of the physical domain index is -1, for the source domain
#define HSTR_SRC_PHYS_DOMAIN -1

// A special value of the logical domain ID, for the source domain
#define HSTR_SRC_LOG_DOMAIN 0

// A special value of the timeout duration is -1, for infinite
#define HSTR_TIME_INFINITE -1

// Maximum sink-side function name size
#define HSTR_MAX_FUNC_NAME_SIZE 80

// COI pipeline function miscellaneous data size
#define HSTR_MISC_DATA_SIZE 4096

// Maximum number of sink-side function arguments
#define HSTR_ARGS_SUPPORTED (HSTR_MISC_DATA_SIZE-HSTR_MAX_FUNC_NAME_SIZE)/sizeof(uint64_t)


#ifdef _WIN32
#       if defined _EXPORT_SYMBOLS
#          define DllAccess  __declspec(dllexport)
#       elif defined _IMPORT_SYMBOLS
#          define DllAccess  __declspec(dllimport)
#       else
#          define DllAccess  /* nothing */
#       endif
#else
#       define DllAccess  /* nothing */
#endif

/////////////////////////////////////////////////////////
// Doxygen settings
/// @file include/hStreams_common.h
///
/// @defgroup hStreams_Common hStreams Common
// TODO: Description for this group
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
///
// hStreams_SetOptions
/// @ingroup hStreams_Common
/// @brief Configure user parameters by setting hStreams Options
///
/// @param  in_options - see the definition of HSTR_OPTIONS
///         [in] HSTR_OPTIONS
///
/// For more details about HSTR_OPTIONS please go directly
///  to HSTR_OPTIONS documentation
///
/// @return HSTR_RESULT_SUCCESS if setting options is successful
///
/// @return HSTR_RESULT_NULL_PTR if in_options is NULL.
///
/// @return HSTR_RESULT_INCONSISTENT_ARGS if options are inconsistent.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_SetOptions(const HSTR_OPTIONS *in_options);

/////////////////////////////////////////////////////////
///
// hStreams_GetCurrentOptions
/// @ingroup hStreams_Common
/// @brief Query user parameters by getting hStreams Options
///
/// hStreams_GetCurrentOptions() copies the current collection of hStreams options from the hStreams library
/// to the buffer provided in a thread safe manner.
///
/// @param  pCurrentOptions
///         [out] The buffer that will receive the current options from the hStreams library.  Must be non-NULL or
///         hStreams_GetCurrentOptions() returns HSTR_RESULT_OUT_OF_RANGE.
///
/// @param  buffSize
///         [in] Indicates the size of the buffer that pCurrentOptions points to.  Must be greater than or equal to
///         sizeof(HSTR_OPTIONS) of hStreams_GetCurrentOptions() returns HSTR_RESULT_OUT_OF_RANGE.
///
/// For more details about HSTR_OPTIONS please go directly
///  to HSTR_OPTIONS documentation
///
/// @return HSTR_RESULT_SUCCESS if getting options is successful
///
/// @return HSTR_RESULT_OUT_OF_RANGE if pCurrentOptions is NULL, or buffSize is less than sizeof(HSTR_OPTIONS)
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT hStreams_GetCurrentOptions(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize);

/////////////////////////////////////////////////////////
///
// hStreams_VersionStringLen
/// @ingroup hStreams_Common
/// @brief Report the length of the version string, including the null termination character
///
/// @param  out_pVersionStringLen
///         [out] The length of the version string, including the null termination character
///
/// @return HSTR_RESULT_NULL_PTR if out_pVersionStringLen is NULL
/// @return HSTR_RESULT_SUCCESS
///
/// @thread_safety Thread safe.
///
//////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetVersionStringLen(uint32_t *out_pVersionStringLen);

/////////////////////////////////////////////////////////
///
// hStreams_Version
/// @ingroup hStreams_Common
/// @brief Report hStreams version info to buffer
///
/// @param  buff
///         [out] The buffer that will receive the version of the hStreams library.
///
/// @param  buffLength
///         [in] The length of the buff parameter.  hStreamsVersion() copies version data
///         upto buffLength bytes to buff.
///
/// @return HSTR_RESULT_SUCCESS
///
/// @return HSTR_RESULT_NULL_PTR if the buffer argument is a NULL pointer
///
/// @return HSTR_RESULT_BUFF_TOO_SMALL when the buffer is too small.
///
/// @thread_safety Thread safe for invocations with different buffer arguments.
///
/// The version string is in format MAJOR.MINOR[.MICRO]. The MICRO-part is ommited if
/// MICRO == 0.
///
//////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Version(char *buff, uint32_t buffLength);

/////////////////////////////////////////////////////////
/// @deprecated This function has been deprecated in favor of
///     hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().
//////////////////////////////////////////////////////////
HSTR_DEPRECATED("hStreams_GetVerbose() has been deprecated. "
                "Please refer to hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().")
DllAccess uint32_t
hStreams_GetVerbose();

/////////////////////////////////////////////////////////
/// @deprecated This function has been deprecated in favor of
///     hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().
//////////////////////////////////////////////////////////
HSTR_DEPRECATED("hStreams_SetVerbose() has been deprecated. "
                "Please refer to hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().")
DllAccess HSTR_RESULT
hStreams_SetVerbose(int target_verbosity);

/////////////////////////////////////////////////////////
///
// hStreams_ResultGetName
/// @ingroup hStreams_Common
/// @brief Get HSTR_RESULT name
///
/// @param  in_olr
///         [in] HSTR_RESULT
///
/// @return Name string
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess const char *hStreams_ResultGetName(
    HSTR_RESULT in_olr);


/////////////////////////////////////////////////////////
///
/// @defgroup hStreams_Common_CPU_MASK CPU_MASK manipulating
/// @ingroup hStreams_Common
/// Functions used for manipulating HSTR_CPU_MASK.
///
/////////////////////////////////////////////////////////

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_ISSET().
static inline uint64_t HSTR_CPU_MASK_ISSET(int bitNumber, const HSTR_CPU_MASK cpu_mask)
{
    if ((size_t)bitNumber < sizeof(HSTR_CPU_MASK) * 8) {
        return ((cpu_mask)[bitNumber / 64] & (((uint64_t)1) << (bitNumber % 64)));
    }
    return 0;
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_SET().
static inline void HSTR_CPU_MASK_SET(int bitNumber, HSTR_CPU_MASK cpu_mask)
{
    if ((size_t)bitNumber < sizeof(HSTR_CPU_MASK) * 8) {
        ((cpu_mask)[bitNumber / 64] |= (((uint64_t)1) << (bitNumber % 64)));
    }
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_ZERO().
static inline void HSTR_CPU_MASK_ZERO(HSTR_CPU_MASK cpu_mask)
{
    memset(cpu_mask, 0, sizeof(HSTR_CPU_MASK));
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_AND().
static inline void HSTR_CPU_MASK_AND(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] & src2[i];
    }
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_XOR().
static inline void HSTR_CPU_MASK_XOR(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] ^ src2[i];
    }
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_OR().
static inline void HSTR_CPU_MASK_OR(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] | src2[i];
    }
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_COUNT().
static inline int HSTR_CPU_MASK_COUNT(const HSTR_CPU_MASK cpu_mask)
{
    int cnt = 0;
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(cpu_mask[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        int cnt_i = 0;
        uint64_t n = cpu_mask[i];

        for (; n; n >>= 1) {
            cnt_i += n & 1l;
        }

        cnt += cnt_i;
    }
    return cnt;
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief Roughly equivalent to CPU_EQUAL().
static inline int HSTR_CPU_MASK_EQUAL(const HSTR_CPU_MASK cpu_mask1, const HSTR_CPU_MASK cpu_mask2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(cpu_mask1[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        if (cpu_mask1[i] != cpu_mask2[i]) {
            return 0;
        }
    }
    return 1;
}

#ifndef _WIN32
/// @ingroup hStreams_Common_CPU_MASK
/// @brief Utility function to translate from cpu_set* to COI_CPU_MASK.
static inline void HSTR_CPU_MASK_XLATE(HSTR_CPU_MASK dest, const cpu_set_t *src)
{
    HSTR_CPU_MASK_ZERO(dest);
    for (unsigned int i = 0; i < sizeof(HSTR_CPU_MASK) / sizeof(dest[0]); ++i) {
        for (unsigned int j = 0; j < 64; ++j) {
            if (CPU_ISSET(i * 64 + j, src)) {
                dest[i] |= ((uint64_t)1) << j;
            }
        }
    }
}

/// @ingroup hStreams_Common_CPU_MASK
/// @brief  Utility function to translate from COI_CPU_MASK to cpu_set*.
static inline void HSTR_CPU_MASK_XLATE_EX(cpu_set_t *dest, const HSTR_CPU_MASK src)
{
    CPU_ZERO(dest);
    for (unsigned int i = 0; i < sizeof(HSTR_CPU_MASK) / sizeof(src[0]); ++i) {
        const uint64_t cpu_mask = src[i];

        for (unsigned int j = 0; j < 64; ++j) {
            const uint64_t bit = ((uint64_t)1) << j;

            if (bit & cpu_mask) {
                CPU_SET(i * 64 + j, dest);
            }
        }
    }
}

#endif //_WIN32

#define CHECK_HSTR_RESULT(func)                                               \
    {                                                                         \
        HSTR_RESULT hret = HSTR_RESULT_SUCCESS;                               \
        hret = func;                                                          \
        if (hret != HSTR_RESULT_SUCCESS) {                                    \
            printf("%s returned %s.\n", #func, hStreams_ResultGetName(hret)); \
            return hret;                                                      \
        }                                                                     \
    }

#ifdef __cplusplus
}
#endif

#endif // __HSTR_COMMON_H__
