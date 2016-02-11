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

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <string.h>

#include "hStreams_MKLWrapper.h"
#include "hStreams_app_api_sink.h"
#include <hStreams_internal.h>
#include "hStreams_Logger.h"

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_memcpy_sink(
    uint64_t byte_len,
    uint64_t *src,
    uint64_t *dest)
{
    memcpy(dest, src, byte_len);
}


HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_memset_sink(
    uint64_t byte_len,
    uint64_t char_value,
    uint64_t *buf)
{
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4244) // possible loss of data
#endif

    memset(buf, char_value, byte_len);
#ifdef _WIN32
#pragma warning(pop)
#endif
}

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_sgemm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11,
    uint64_t arg_in12, uint64_t arg_in13)
{

    const CBLAS_ORDER     arg0  = (CBLAS_ORDER)arg_in0;         //Order
    const CBLAS_TRANSPOSE arg1  = (CBLAS_TRANSPOSE)arg_in1;     //TransA
    const CBLAS_TRANSPOSE arg2  = (CBLAS_TRANSPOSE)arg_in2;     //TransB
    const int64_t         arg3  = (int64_t)arg_in3;             //M
    const int64_t         arg4  = (int64_t)arg_in4;             //N
    const int64_t         arg5  = (int64_t)arg_in5;             //K
    uint64_t              arg6  = (uint64_t)arg_in6;            //alpha
    const int64_t         arg7  = (int64_t)arg_in7;             //lda
    const int64_t         arg8  = (int64_t)arg_in8;             //ldb
    uint64_t              arg9  = (uint64_t)arg_in9;            //beta
    const int64_t         arg10 = (int64_t)arg_in10;            //ldc
    const float          *arg11 = (float *)arg_in11;            //A
    const float          *arg12 = (float *)arg_in12;            //B
    float                *arg13 = (float *)arg_in13;            //C

    floatToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(arg6);
    uBeta.Set_uint64_t(arg9);

    MKLWrapper::cblas_sgemm(arg0, arg1, arg2, arg3, arg4, arg5,
                            uAlpha.Get(), arg11, arg7, arg12, arg8, uBeta.Get(), arg13, arg10);

    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE) << "Completed remote hStreams_sgemm_sink";
}

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_dgemm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11,
    uint64_t arg_in12, uint64_t arg_in13)
{

    const CBLAS_ORDER     arg0  = (CBLAS_ORDER)arg_in0;         //Order
    const CBLAS_TRANSPOSE arg1  = (CBLAS_TRANSPOSE)arg_in1;     //TransA
    const CBLAS_TRANSPOSE arg2  = (CBLAS_TRANSPOSE)arg_in2;     //TransB
    const int64_t         arg3  = (int64_t)arg_in3;             //M
    const int64_t         arg4  = (int64_t)arg_in4;             //N
    const int64_t         arg5  = (int64_t)arg_in5;             //K
    uint64_t              arg6  = (uint64_t)arg_in6;            //alpha
    const int64_t         arg7  = (int64_t)arg_in7;             //lda
    const int64_t         arg8  = (int64_t)arg_in8;             //ldb
    uint64_t              arg9  = (uint64_t)arg_in9;            //beta
    const int64_t         arg10 = (int64_t)arg_in10;            //ldc
    const double         *arg11 = (double *)arg_in11;           //A
    const double         *arg12 = (double *)arg_in12;           //B
    double               *arg13 = (double *)arg_in13;           //C

    doubleToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(arg6);
    uBeta.Set_uint64_t(arg9);

    MKLWrapper::cblas_dgemm(arg0, arg1, arg2, arg3, arg4, arg5,
                            uAlpha.Get(), arg11, arg7, arg12, arg8, uBeta.Get(), arg13, arg10);
    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE) << "Completed remote hStreams_dgemm_sink";
}

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_cgemm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11,
    uint64_t arg_in12, uint64_t arg_in13)
{

    const CBLAS_ORDER     arg0  = (CBLAS_ORDER)arg_in0;         //Order
    const CBLAS_TRANSPOSE arg1  = (CBLAS_TRANSPOSE)arg_in1;     //TransA
    const CBLAS_TRANSPOSE arg2  = (CBLAS_TRANSPOSE)arg_in2;     //TransB
    const int64_t         arg3  = (int64_t)arg_in3;             //M
    const int64_t         arg4  = (int64_t)arg_in4;             //N
    const int64_t         arg5  = (int64_t)arg_in5;             //K
    uint64_t              arg6  = (uint64_t)arg_in6;            //alpha
    const int64_t         arg7  = (int64_t)arg_in7;             //lda
    const int64_t         arg8  = (int64_t)arg_in8;             //ldb
    uint64_t              arg9  = (uint64_t)arg_in9;            //beta
    const int64_t         arg10 = (int64_t)arg_in10;            //ldc
    const void           *arg11 = (void *)arg_in11;             //A
    const void           *arg12 = (void *)arg_in12;             //B
    void                 *arg13 = (void *)arg_in13;             //C

    MKL_Complex8ToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(arg6);
    uBeta.Set_uint64_t(arg9);

    MKLWrapper::cblas_cgemm(arg0, arg1, arg2, arg3, arg4, arg5,
                            &uAlpha.Get(), arg11, arg7, arg12, arg8, &uBeta.Get(), arg13, arg10);
    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE) << "Completed remote hStreams_cgemm_sink";
}

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_zgemm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11,
    uint64_t arg_in12, uint64_t arg_in13, uint64_t arg_in14, uint64_t arg_in15)
{

    const CBLAS_ORDER     arg0  = (CBLAS_ORDER)arg_in0;         //Order
    const CBLAS_TRANSPOSE arg1  = (CBLAS_TRANSPOSE)arg_in1;     //TransA
    const CBLAS_TRANSPOSE arg2  = (CBLAS_TRANSPOSE)arg_in2;     //TransB
    const int64_t         arg3  = (int64_t)arg_in3;             //M
    const int64_t         arg4  = (int64_t)arg_in4;             //N
    const int64_t         arg5  = (int64_t)arg_in5;             //K
    uint64_t              arg6  = (uint64_t)arg_in6;            //alpha[0]
    uint64_t              arg7  = (uint64_t)arg_in7;            //alpha[1]
    const int64_t         arg8  = (int64_t)arg_in8;             //lda
    const int64_t         arg9  = (int64_t)arg_in9;             //ldb
    uint64_t              arg10 = (uint64_t)arg_in10;           //beta[0]
    uint64_t              arg11 = (uint64_t)arg_in11;           //beta[1]
    const int64_t         arg12 = (int64_t)arg_in12;            //ldc
    const void           *arg13 = (void *)arg_in13;             //A
    const void           *arg14 = (void *)arg_in14;             //B
    void                 *arg15 = (void *)arg_in15;             //C

    MKL_Complex16ToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(arg6, 0);
    uAlpha.Set_uint64_t(arg7, 1);
    uBeta.Set_uint64_t(arg10, 0);
    uBeta.Set_uint64_t(arg11, 1);

    MKLWrapper::cblas_zgemm(arg0, arg1, arg2, arg3, arg4, arg5,
                            &uAlpha.Get(), arg13, arg8, arg14, arg9, &uBeta.Get(), arg15, arg12);
    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE) << "Completed remote hStreams_zgemm_sink";
}
