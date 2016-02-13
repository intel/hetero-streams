/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#include <stdio.h>
#include <string.h>
#include <mkl.h>
#include <hStreams_common.h>
#include "hStreams_custom.h"
#include <type_converter.h>
#ifdef __cplusplus
extern "C" {
#endif

void hStreams_custom_dtrsm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11)
{


    const CBLAS_ORDER Order = (CBLAS_ORDER)arg_in0;
    const CBLAS_SIDE Side = (CBLAS_SIDE)arg_in1;
    const CBLAS_UPLO Uplo = (CBLAS_UPLO)arg_in2;
    const CBLAS_TRANSPOSE TransA = (CBLAS_TRANSPOSE)arg_in3;
    const CBLAS_DIAG Diag = (CBLAS_DIAG)arg_in4;

    const MKL_INT M = (MKL_INT)arg_in5;
    const MKL_INT N = (MKL_INT)arg_in6;
    uint64_t alpha = (uint64_t)arg_in7;
    const MKL_INT lda = (MKL_INT)arg_in8;
    const MKL_INT ldb = (MKL_INT)arg_in9;
    const double *A = (double *)arg_in10;
    double *B = (double *)arg_in11;

    doubleToUint64_t uAlpha;

    uAlpha.Set_uint64_t(alpha);

    cblas_dtrsm(Order, Side, Uplo, TransA, Diag, M, N, uAlpha.Get(), A, lda, B, ldb);

}

void hStreams_custom_dpotrf_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4)
{

    int matrix_order = (int)arg_in0;
    uint64_t uplo = (uint64_t)arg_in1;
    lapack_int n = (lapack_int)arg_in2;
    lapack_int lda = (lapack_int)arg_in3;
    double *a = (double *)arg_in4;

    charToUint64_t uUplo;

    uUplo.Set_uint64_t(uplo);

    int info = LAPACKE_dpotrf(matrix_order, uUplo.Get(), n, a, lda);

}

void hStreams_custom_dsyrk_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10)
{


    const CBLAS_ORDER Order = (CBLAS_ORDER)arg_in0;
    const CBLAS_UPLO Uplo = (CBLAS_UPLO)arg_in1;
    const CBLAS_TRANSPOSE Trans = (CBLAS_TRANSPOSE)arg_in2;
    const MKL_INT N = (MKL_INT)arg_in3;
    const MKL_INT K = (MKL_INT)arg_in4;

    uint64_t alpha = (uint64_t)arg_in5;
    const MKL_INT lda = (MKL_INT)arg_in6;
    uint64_t beta = (uint64_t)arg_in7;

    const MKL_INT ldc = (MKL_INT)arg_in8;

    const double *A = (double *)arg_in9;
    double *C = (double *)arg_in10;

    doubleToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(alpha);
    uBeta.Set_uint64_t(beta);

    cblas_dsyrk(Order, Uplo, Trans, N, K, uAlpha.Get(), A, lda, uBeta.Get(), C, ldc);

}

void hStreams_custom_dgetrf_square_nopiv_colMaj_sink(
    uint64_t arg_in0,  uint64_t arg_in1)
{
    int size = (int)arg_in0;
    double *mat = (double *)arg_in1;

    double invDiag;

    #pragma omp parallel
    {
        int i, j, k, ivec;
        double factor;
        int off1, off2;

        for (k = 0; k < size; ++k) {
            off1 = k * size;
            invDiag = 1 / mat[off1 + k];

            #pragma omp for simd
            for (i = k + 1; i < size; ++i)
                //get L entries below the diagonal
            {
                mat[off1 + i] = invDiag * mat[off1 + i];
            }

            //get Schur Complement
            #pragma omp for
            for (j = k + 1; j < size; ++j) {
                off2 = j * size;
                factor = mat[off2 + k];
#pragma simd
                for (i = k + 1; i < size; ++i) {
                    mat[off2 + i] -= factor * mat[off1 + i];
                }
            }
        }
    }
}

void hStreams_custom_dgetrf_square_nopiv_rowMaj_sink(
    uint64_t arg_in0,  uint64_t arg_in1)
{
    int size = (int)arg_in0;
    double *mat = (double *)arg_in1;

    double invDiag;

    #pragma omp parallel
    {
        int i, j, k, ivec;
        double factor;
        int off1, off2;

        for (k = 0; k < size; ++k) {
            off1 = k * size;
            invDiag = 1 / mat[off1 + k];

            #pragma omp for
            for (i = k + 1; i < size; ++i) {
                off2 = i * size;
                //get L entries below the diagonal
                mat[off2 + k] = invDiag * mat[off2 + k];

                //get Schur Complement
                factor =  mat[off2 + k];
#pragma simd
                for (j = k + 1; j < size; ++j) {
                    mat[off2 + j] -= factor * mat[off1 + j];
                }
            }
        }
    }
}

void hStreams_dgemm_sink(
    uint64_t arg_in0,  uint64_t arg_in1,  uint64_t arg_in2,  uint64_t arg_in3,
    uint64_t arg_in4,  uint64_t arg_in5,  uint64_t arg_in6,  uint64_t arg_in7,
    uint64_t arg_in8,  uint64_t arg_in9,  uint64_t arg_in10, uint64_t arg_in11,
    uint64_t arg_in12, uint64_t arg_in13)
{

    const CBLAS_ORDER     arg0  = (CBLAS_ORDER)arg_in0;         //Order
    const CBLAS_TRANSPOSE arg1  = (CBLAS_TRANSPOSE)arg_in1;     //TransA
    const CBLAS_TRANSPOSE arg2  = (CBLAS_TRANSPOSE)arg_in2;     //TransB
    const MKL_INT         arg3  = (MKL_INT)arg_in3;             //M
    const MKL_INT         arg4  = (MKL_INT)arg_in4;             //N
    const MKL_INT         arg5  = (MKL_INT)arg_in5;             //K
    uint64_t              arg6  = (uint64_t)arg_in6;            //alpha
    const MKL_INT         arg7  = (MKL_INT)arg_in7;             //lda
    const MKL_INT         arg8  = (MKL_INT)arg_in8;             //ldb
    uint64_t              arg9  = (uint64_t)arg_in9;            //beta
    const MKL_INT         arg10 = (MKL_INT)arg_in10;            //ldc
    const double         *arg11 = (double *)arg_in11;           //A
    const double         *arg12 = (double *)arg_in12;           //B
    double               *arg13 = (double *)arg_in13;           //C

    doubleToUint64_t uAlpha, uBeta;

    uAlpha.Set_uint64_t(arg6);
    uBeta.Set_uint64_t(arg9);

    cblas_dgemm(arg0, arg1, arg2, arg3, arg4, arg5,
                uAlpha.Get(), arg11, arg7, arg12, arg8, uBeta.Get(), arg13, arg10);
}
void hStreams_memset_sink(
    uint64_t byte_len,
    uint64_t char_value,
    uint64_t *buf)
{
    memset(buf, char_value, byte_len);
}
#ifdef __cplusplus
}
#endif
