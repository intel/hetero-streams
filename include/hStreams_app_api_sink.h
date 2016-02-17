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

#ifndef __HSTR_APP_API_SINK_H__

#define __HSTR_APP_API_SINK_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "hStreams_version.h"
#include "hStreams_types.h"
#include "hStreams_internal.h"
#include "hStreams_sink.h"
#endif //DOXYGEN_SHOULD_SKIP_THIS


#ifdef __cplusplus
extern "C" {
#endif

// //////////////////////////////////////////////////////////////////////
// These are declarations of the convenience functions referenced
//   by hStreams_app_api_sink.cpp
// These functions may be called from user-defined sink-side functions
// Notice that these conform to requirements for remotely-invoked functions:
//   arguments are ordered so that scalar arguments precede heap args
//   all arguments are 64 bits
// The functions are:
//  hStreams_memcpy_sink
//  hStreams_memset_sink
//  hStreams_sgemm_sink
//  hStreams_dgemm_sink
//  hStreams_cgemm_sink
//  hStreams_zgemm_sink

/////////////////////////////////////////////////////////
// Doxygen settings
/// @file include/hStreams_app_api_sink.h
///
/// @defgroup hStreams_AppApiSink hStreams AppApiSink
// TODO: Description for this group
//
/////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
///
// hStreams_memcpy_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls memcpy from string.h from (remote) sink side.
///
///  For use on sink side only
///
/// @param byte_len
///        [in] number of bytes to copy
///
/// @param src
///        [in] source address to copy from
///
/// @param dest
///        [in] destination address to copy to
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_memcpy_sink(
    uint64_t byte_len,
    uint64_t *src,
    uint64_t *dest);

//////////////////////////////////////////////////////////////////
///
// hStreams_memset_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls memset from string.h from (remote) sink side.
///
///  For use on sink side only
///
/// @param byte_len
///        [in] number of bytes to copy
///
/// @param char_value
///        [in] character-sized value to fill with
///
/// @param buf
///        [in] starting address to fill at
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_memset_sink(
    uint64_t byte_len,
    uint64_t char_value,
    uint64_t *buf);

//////////////////////////////////////////////////////////////////
///
// hStreams_sgemm_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls sgemm from (remote) sink side.
///
///  - For use on sink side only
///
/// @param arg0
///        [in] const CBLAS_ORDER arg0,      Order
///
/// @param arg1
///        [in] const CBLAS_TRANSPOSE arg1,  TransA
///
/// @param arg2
///        [in] const CBLAS_TRANSPOSE arg2,  TransB
///
/// @param arg3
///        [in] const MKL_INT arg3,          M
///
/// @param arg4
///        [in] const MKL_INT arg4,          N
///
/// @param arg5
///        [in] const MKL_INT arg5,          K
///
/// @param arg6
///        [in] float arg6,                  alpha (actually a float passed in a uint64_t)
///
/// @param arg7
///        [in] const MKL_INT arg7,          lda
///
/// @param arg8
///        [in] const MKL_INT arg8,          ldb
///
/// @param arg9
///        [in] float arg9,                  beta (actually a float passed in a uint64_t)
///
/// @param arg10
///        [in] const MKL_INT arg10,         ldc
///
/// @param arg11
///        [in] const float *arg11,          A
///
/// @param arg12
///        [in] const float *arg12,          B
///
/// @param arg13
///        [in] float *arg13);               C
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_sgemm_sink(
    uint64_t arg0 , //  const CBLAS_ORDER arg0,      //Order
    uint64_t arg1 , //  const CBLAS_TRANSPOSE arg1,  //TransA
    uint64_t arg2 , //  const CBLAS_TRANSPOSE arg2,  //TransB
    uint64_t arg3 , //  const MKL_INT arg3,          //M
    uint64_t arg4 , //  const MKL_INT arg4,          //N
    uint64_t arg5 , //  const MKL_INT arg5,          //K
    uint64_t arg6 , //  float arg6,                  //alpha
    uint64_t arg7 , //  const MKL_INT arg7,          //lda
    uint64_t arg8 , //  const MKL_INT arg8,          //ldb
    uint64_t arg9 , //  float arg9,                  //beta
    uint64_t arg10, //  const MKL_INT arg10,         //ldc
    uint64_t arg11, //  const float *arg11,          //A
    uint64_t arg12, //  const float *arg12,          //B
    uint64_t arg13);//  float *arg13);               //C

//////////////////////////////////////////////////////////////////
///
// hStreams_dgemm_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls dgemm from (remote) sink side.
///
///  - For use on sink side only
///
/// @param arg0
///        [in] const CBLAS_ORDER arg0,      Order
///
/// @param arg1
///        [in] const CBLAS_TRANSPOSE arg1,  TransA
///
/// @param arg2
///        [in] const CBLAS_TRANSPOSE arg2,  TransB
///
/// @param arg3
///        [in] const MKL_INT arg3,          M
///
/// @param arg4
///        [in] const MKL_INT arg4,          N
///
/// @param arg5
///        [in] const MKL_INT arg5,          K
///
/// @param arg6
///        [in] double arg6,                 alpha (actually a double passed in a uint64_t)
///
/// @param arg7
///        [in] const MKL_INT arg7,          lda
///
/// @param arg8
///        [in] const MKL_INT arg8,          ldb
///
/// @param arg9
///        [in] double arg9,                 beta (actually a double passed in a uint64_t)
///
/// @param arg10
///        [in] const MKL_INT arg10,         ldc
///
/// @param arg11
///        [in] const double *arg11,         A
///
/// @param arg12
///        [in] const double *arg12,         B
///
/// @param arg13
///        [in] double *arg13);              C
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_dgemm_sink(
    uint64_t arg0 , //    const CBLAS_ORDER arg0,      //Order
    uint64_t arg1 , //    const CBLAS_TRANSPOSE arg1,  //TransA
    uint64_t arg2 , //    const CBLAS_TRANSPOSE arg2,  //TransB
    uint64_t arg3 , //    const MKL_INT arg3,          //M
    uint64_t arg4 , //    const MKL_INT arg4,          //N
    uint64_t arg5 , //    const MKL_INT arg5,          //K
    uint64_t arg6 , //    double arg6,                 //alpha (actually a double passed in a uint64_t)
    uint64_t arg7 , //    const MKL_INT arg7,          //lda
    uint64_t arg8 , //    const MKL_INT arg8,          //ldb
    uint64_t arg9 , //    double arg9,                 //beta (actually a double passed in a uint64_t)
    uint64_t arg10, //    const MKL_INT arg10,         //ldc
    uint64_t arg11, //    const double *arg11,         //A
    uint64_t arg12, //    const double *arg12,         //B
    uint64_t arg13);//    double *arg13);              //C

//////////////////////////////////////////////////////////////////
///
// hStreams_cgemm_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls cgemm from (remote) sink side.
///
///  - For use on sink side only
///
/// @param arg0
///        [in] const CBLAS_ORDER arg0,      Order
///
/// @param arg1
///        [in] const CBLAS_TRANSPOSE arg1,  TransA
///
/// @param arg2
///        [in] const CBLAS_TRANSPOSE arg2,  TransB
///
/// @param arg3
///        [in] const MKL_INT arg3,          M
///
/// @param arg4
///        [in] const MKL_INT arg4,          N
///
/// @param arg5
///        [in] const MKL_INT arg5,          K
///
/// @param arg6
///        [in] MKL_Complex8 arg6,           alpha (actually a MKL_Complex8 passed in a uint64_t)
///
/// @param arg7
///        [in] const MKL_INT arg7,          lda
///
/// @param arg8
///        [in] const MKL_INT arg8,          ldb
///
/// @param arg9
///        [in] MKL_Complex8 arg9            beta (actually a MKL_Complex8 passed in a uint64_t)
///
/// @param arg10
///        [in] const MKL_INT arg10,         ldc
///
/// @param arg11
///        [in] const void  *arg11,          A
///
/// @param arg12
///        [in] const void  *arg12,          B
///
/// @param arg13
///        [in] void  *arg13);               C
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_cgemm_sink(
    uint64_t arg0 , //    const CBLAS_ORDER arg0,      //Order
    uint64_t arg1 , //    const CBLAS_TRANSPOSE arg1,  //TransA
    uint64_t arg2 , //    const CBLAS_TRANSPOSE arg2,  //TransB
    uint64_t arg3 , //    const MKL_INT arg3,          //M
    uint64_t arg4 , //    const MKL_INT arg4,          //N
    uint64_t arg5 , //    const MKL_INT arg5,          //K
    uint64_t arg6 , //    MKL_Complex8 arg6,           //alpha (actually a MKL_Complex8 passed in a uint64_t)
    uint64_t arg7 , //    const MKL_INT arg7,          //lda
    uint64_t arg8 , //    const MKL_INT arg8,          //ldb
    uint64_t arg9 , //    MKL_Complex8 arg9,           //beta (actually a MKL_Complex8 passed in a uint64_t)
    uint64_t arg10, //    const MKL_INT arg10,         //ldc
    uint64_t arg11, //    const void *arg11,           //A
    uint64_t arg12, //    const void *arg12,           //B
    uint64_t arg13);//    void *arg13);                //C
//////////////////////////////////////////////////////////////////
///
// hStreams_zgemm_sink
/// @ingroup hStreams_AppApiSink
/// @brief Calls zgemm from (remote) sink side.
///
///  - For use on sink side only
///
/// @param arg0
///        [in] const CBLAS_ORDER arg0,      Order
///
/// @param arg1
///        [in] const CBLAS_TRANSPOSE arg1,  TransA
///
/// @param arg2
///        [in] const CBLAS_TRANSPOSE arg2,  TransB
///
/// @param arg3
///        [in] const MKL_INT arg3,          M
///
/// @param arg4
///        [in] const MKL_INT arg4,          N
///
/// @param arg5
///        [in] const MKL_INT arg5,          K
///
/// @param arg6
///        [in] MKL_Complex16 in arg6 and arg7,       alpha (actually a MKL_Complex16 passed in two uint64_t's)
///             and arg7,
///
/// @param  arg7
///        [in] MKL_Complex16 in arg6 and arg7,       alpha (actually a MKL_Complex16 passed in two uint64_t's)
///             and arg7,
///
/// @param arg8
///        [in] const MKL_INT arg8,          lda
///
/// @param arg9
///        [in] const MKL_INT arg9,          ldb
///
/// @param arg10
///        [in] MKL_Complex16 in arg10 and arg11,      beta (actually a MKL_Complex16 passed in two uint64_t's)
///             and arg11,
///
/// @param arg11
///        [in] MKL_Complex16 in arg10 and arg11,      beta (actually a MKL_Complex16 passed in two uint64_t's)
///             and arg11,
///
/// @param arg12
///        [in] const MKL_INT arg12,         ldc
///
/// @param arg13
///        [in] const void  *arg13,          A
///
/// @param arg14
///        [in] const void  *arg14,          B
///
/// @param arg15
///        [in] void  *arg15);               C
///
/// @return void
///
/// @thread_safety Thread safe for calls on different data.
///
//////////////////////////////////////////////////////////////////
HSTREAMS_EXPORT
void hStreams_zgemm_sink(
    uint64_t arg0 , //    const CBLAS_ORDER arg0,      //Order
    uint64_t arg1 , //    const CBLAS_TRANSPOSE arg1,  //TransA
    uint64_t arg2 , //    const CBLAS_TRANSPOSE arg2,  //TransB
    uint64_t arg3 , //    const MKL_INT arg3,          //M
    uint64_t arg4 , //    const MKL_INT arg4,          //N
    uint64_t arg5 , //    const MKL_INT arg5,          //K
    uint64_t arg6 , //    MKL_Complex16 arg6,               //alpha (actually a MKL_Complex16 passed in two uint64_t's)
    uint64_t arg7 , //        and arg7,                //alpha
    uint64_t arg8 , //    const MKL_INT arg8,          //lda
    uint64_t arg9 , //    const MKL_INT arg9,          //ldb
    uint64_t arg10, //    MKL_Complex16  arg10,        //beta (actually a MKL_Complex16 passed in two uint64_t's)
    uint64_t arg11, //        and arg11,               //beta
    uint64_t arg12, //    const MKL_INT arg12,         //ldc
    uint64_t arg13, //    const void *arg11,           //A
    uint64_t arg14, //    const void *arg12,           //B
    uint64_t arg15);//    void *arg13);                //C


#ifdef __cplusplus
}
#endif

#endif // __HSTR_APP_API_SINK_H__
