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

#ifndef HSTREAMS_MKL_WRAPPER_H
#define HSTREAMS_MKL_WRAPPER_H

#include <hStreams_types.h>
#include <mkl.h>

class MKLWrapper
{
    // pointers to LP64 *gemm
    typedef void (*cblas_sgemm_lp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int32_t,           // M
        const int32_t,           // N
        const int32_t,           // K
        const float,             // alpha
        const float *,           // A
        const int32_t,           // lda
        const float *,           // B
        const int32_t,           // ldb
        const float,             // beta
        float *,                 // C
        const int32_t);          // ldc

    typedef void (*cblas_dgemm_lp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int32_t,           // M
        const int32_t,           // N
        const int32_t,           // K
        const double,            // alpha
        const double *,          // A
        const int32_t,           // lda
        const double *,          // B
        const int32_t,           // ldb
        const double,            // beta
        double *,                // C
        const int32_t);          // ldc

    typedef void (*cblas_cgemm_lp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int32_t,           // M
        const int32_t,           // N
        const int32_t,           // K
        const void *,            // alpha
        const void *,            // A
        const int32_t,           // lda
        const void *,            // B
        const int32_t,           // ldb
        const void *,            // beta
        void *,                  // C
        const int32_t);          // ldc

    typedef void (*cblas_zgemm_lp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int32_t,           // M
        const int32_t,           // N
        const int32_t,           // K
        const void *,            // alpha
        const void *,            // A
        const int32_t,           // lda
        const void *,            // B
        const int32_t,           // ldb
        const void *,            // beta
        void *,                  // C
        const int32_t);          // ldc


    // pointers to ILP64 *gemm
    typedef void (*cblas_sgemm_ilp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int64_t,           // M
        const int64_t,           // N
        const int64_t,           // K
        const float,             // alpha
        const float *,           // A
        const int64_t,           // lda
        const float *,           // B
        const int64_t,           // ldb
        const float,             // beta
        float *,                 // C
        const int64_t);          // ldc

    typedef void (*cblas_dgemm_ilp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int64_t,           // M
        const int64_t,           // N
        const int64_t,           // K
        const double,            // alpha
        const double *,          // A
        const int64_t,           // lda
        const double *,          // B
        const int64_t,           // ldb
        const double,            // beta
        double *,                // C
        const int64_t);          // ldc

    typedef void (*cblas_cgemm_ilp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int64_t,           // M
        const int64_t,           // N
        const int64_t,           // K
        const void *,            // alpha
        const void *,            // A
        const int64_t,           // lda
        const void *,            // B
        const int64_t,           // ldb
        const void *,            // beta
        void *,                  // C
        const int64_t);          // ldc

    typedef void (*cblas_zgemm_ilp64_t)(
        const CBLAS_ORDER,       // Order
        const CBLAS_TRANSPOSE,   // TransA
        const CBLAS_TRANSPOSE,   // TransB
        const int64_t,           // M
        const int64_t,           // N
        const int64_t,           // K
        const void *,            // alpha
        const void *,            // A
        const int64_t,           // lda
        const void *,            // B
        const int64_t,           // ldb
        const void *,            // beta
        void *,                  // C
        const int64_t);          // ldc


public:
    static void *cblas_sgemm_handler;
    static void *cblas_dgemm_handler;
    static void *cblas_cgemm_handler;
    static void *cblas_zgemm_handler;

    static void cblas_sgemm(
        const CBLAS_ORDER Order,
        const CBLAS_TRANSPOSE TransA,
        const CBLAS_TRANSPOSE TransB,
        const int64_t M,
        const int64_t N,
        const int64_t K,
        const float alpha,
        const float *A,
        const int64_t lda,
        const float *B,
        const int64_t ldb,
        const float beta,
        float *C,
        const int64_t ldc);

    static void cblas_dgemm(
        const CBLAS_ORDER Order,
        const CBLAS_TRANSPOSE TransA,
        const CBLAS_TRANSPOSE TransB,
        const int64_t M,
        const int64_t N,
        const int64_t K,
        const double alpha,
        const double *A,
        const int64_t lda,
        const double *B,
        const int64_t ldb,
        const double beta,
        double *C,
        const int64_t ldc);

    static void cblas_cgemm(
        const CBLAS_ORDER Order,
        const CBLAS_TRANSPOSE TransA,
        const CBLAS_TRANSPOSE TransB,
        const int64_t M,
        const int64_t N,
        const int64_t K,
        const void *alpha,
        const void *A,
        const int64_t lda,
        const void *B,
        const int64_t ldb,
        const void *beta,
        void *C,
        const int64_t ldc);

    static void cblas_zgemm(
        const CBLAS_ORDER Order,
        const CBLAS_TRANSPOSE TransA,
        const CBLAS_TRANSPOSE TransB,
        const int64_t M,
        const int64_t N,
        const int64_t K,
        const void *alpha,
        const void *A,
        const int64_t lda,
        const void *B,
        const int64_t ldb,
        const void *beta,
        void *C,
        const int64_t ldc);
};

#endif

