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

#include <limits>

#include "hStreams_MKLWrapper.h"
#include "hStreams_Logger.h"
#include "hStreams_internal_vars_common.h"

#ifdef _WIN32
#undef max // Disable max() macro to use std::numeric_limits<T>::max()
#endif

void *MKLWrapper::cblas_sgemm_handler = NULL;
void *MKLWrapper::cblas_dgemm_handler = NULL;
void *MKLWrapper::cblas_cgemm_handler = NULL;
void *MKLWrapper::cblas_zgemm_handler = NULL;

static void warnIfTruncated(std::string func_name, int64_t M, int64_t N, int64_t K, int64_t lda, int64_t ldb, int64_t ldc)
{
    if (M > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << M << " for argument \"M\" of " << func_name << " has been truncated to " << (int32_t) M;
    }
    if (N > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << N << " for argument \"N\" of " << func_name << " has been truncated to " << (int32_t) N;
    }
    if (K > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << K << " for argument \"K\" of " << func_name << " has been truncated to " << (int32_t) K;
    }
    if (lda > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << lda << " for argument \"lda\" of " << func_name << " has been truncated to " << (int32_t) lda;
    }
    if (ldb > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << ldb << " for argument \"ldb\" of " << func_name << " has been truncated to " << (int32_t) ldb;
    }
    if (ldc > std::numeric_limits<int32_t>::max()) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Value " << ldc << " for argument \"ldc\" of " << func_name << " has been truncated to " << (int32_t) ldc;
    }
}

void MKLWrapper::cblas_sgemm(
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
    const int64_t ldc)
{
    switch (globals::mkl_interface) {
    case HSTR_MKL_LP64:
        warnIfTruncated("cblas_sgemm", M, N, K, lda, ldb, ldc);
        ((cblas_sgemm_lp64_t)cblas_sgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;

    case HSTR_MKL_ILP64:
        ((cblas_sgemm_ilp64_t)cblas_sgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;
    }
}

void MKLWrapper::cblas_dgemm(
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
    const int64_t ldc)
{
    switch (globals::mkl_interface) {
    case HSTR_MKL_LP64:
        warnIfTruncated("cblas_dgemm", M, N, K, lda, ldb, ldc);
        ((cblas_dgemm_lp64_t)cblas_dgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;

    case HSTR_MKL_ILP64:
        ((cblas_dgemm_ilp64_t)cblas_dgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;
    }

}

void MKLWrapper::cblas_cgemm(
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
    const int64_t ldc)
{
    switch (globals::mkl_interface) {
    case HSTR_MKL_LP64:
        warnIfTruncated("cblas_cgemm", M, N, K, lda, ldb, ldc);
        ((cblas_cgemm_lp64_t)cblas_cgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;

    case HSTR_MKL_ILP64:
        ((cblas_cgemm_ilp64_t)cblas_cgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;
    }
}

void MKLWrapper::cblas_zgemm(
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
    const int64_t ldc)
{
    switch (globals::mkl_interface) {
    case HSTR_MKL_LP64:
        warnIfTruncated("cblas_zgemm", M, N, K, lda, ldb, ldc);
        ((cblas_zgemm_lp64_t)cblas_zgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;

    case HSTR_MKL_ILP64:
        ((cblas_zgemm_ilp64_t)cblas_zgemm_handler)(
            Order, TransA, TransB,
            M, N, K,
            alpha,
            A, lda, B, ldb,
            beta,
            C, ldc);
        break;
    }
}
