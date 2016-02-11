/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#include "hStreams_custom.h"
#include "type_converter.h"
#include <hStreams_source.h>
#include <hStreams_common.h>
#include <mkl.h>
#define HSTR_MAX_THREADS 500

HSTR_RESULT
hStreams_custom_dtrsm(const CBLAS_ORDER Order, const CBLAS_SIDE Side,
                      const CBLAS_UPLO Uplo, const CBLAS_TRANSPOSE TransA,
                      const CBLAS_DIAG Diag, const MKL_INT M, const MKL_INT N,
                      const double alpha, const double *A, const MKL_INT lda,
                      double *B, const MKL_INT ldb, const uint32_t in_LogicalStr,
                      HSTR_EVENT *out_pEvent)
{
    doubleToUint64_t uAlpha;

    uAlpha.Set(alpha);

    uint64_t args[12];
    // Set up scalar args, then heap args
    args[ 0] = (uint64_t)(Order);
    args[ 1] = (uint64_t)(Side);
    args[ 2] = (uint64_t)(Uplo);
    args[ 3] = (uint64_t)(TransA);
    args[ 4] = (uint64_t)(Diag);
    args[ 5] = (uint64_t)(M);
    args[ 6] = (uint64_t)(N);
    args[ 7] = (uint64_t)(uAlpha.Get_uint64_t());
    args[ 8] = (uint64_t)(lda);
    args[ 9] = (uint64_t)(ldb);
    args[10] = (uint64_t)(A);
    args[11] = (uint64_t)(B);


    CHECK_HSTR_RESULT(
        hStreams_EnqueueCompute(
            in_LogicalStr,
            "hStreams_custom_dtrsm_sink",
            10,            // scalar args
            2,             // heap args
            args,          // arg array
            out_pEvent,    // event
            NULL,          // return value pointer
            0));           // return value size

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_dpotrf(int matrix_order, char uplo, lapack_int n,
                       double *a, lapack_int lda, const uint32_t in_LogicalStr,
                       HSTR_EVENT *out_pEvent)
{
    charToUint64_t uUplo;

    uUplo.Set(uplo);

    uint64_t args[5];
    // Set up scalar args, then heap args
    args[ 0] = (uint64_t)(matrix_order);
    args[ 1] = (uint64_t)(uUplo.Get_uint64_t());
    args[ 2] = (uint64_t)(n);
    args[ 3] = (uint64_t)(lda);
    args[ 4] = (uint64_t)(a);

    CHECK_HSTR_RESULT(
        hStreams_EnqueueCompute(
            in_LogicalStr,
            "hStreams_custom_dpotrf_sink",
            4,            // scalar args
            1,             // heap args
            args,          // arg array
            out_pEvent,    // event
            NULL,          // return value pointer
            0));           // return value size

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_dsyrk(const CBLAS_ORDER Order, const CBLAS_UPLO Uplo,
                      const CBLAS_TRANSPOSE Trans, const MKL_INT N, const MKL_INT K,
                      const double alpha, const double *A, const MKL_INT lda,
                      const double beta, double *C, const MKL_INT ldc,
                      const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent)
{
    doubleToUint64_t uAlpha, uBeta;

    uAlpha.Set(alpha);
    uBeta.Set(beta);

    uint64_t args[11];
    // Set up scalar args, then heap args
    args[ 0] = (uint64_t)(Order);
    args[ 1] = (uint64_t)(Uplo);
    args[ 2] = (uint64_t)(Trans);
    args[ 3] = (uint64_t)(N);
    args[ 4] = (uint64_t)(K);
    args[ 5] = (uint64_t)(uAlpha.Get_uint64_t());
    args[ 6] = (uint64_t)(lda);
    args[ 7] = (uint64_t)(uBeta.Get_uint64_t());
    args[ 8] = (uint64_t)(ldc);
    args[ 9] = (uint64_t)(A);
    args[10] = (uint64_t)(C);


    CHECK_HSTR_RESULT(
        hStreams_EnqueueCompute(
            in_LogicalStr,
            "hStreams_custom_dsyrk_sink",
            9,            // scalar args
            2,             // heap args
            args,          // arg array
            out_pEvent,    // event
            NULL,          // return value pointer
            0));           // return value size
    return HSTR_RESULT_SUCCESS;

}

HSTR_RESULT
hStreams_custom_dgetrf_square_nopiv_rowMaj(int size, double *mat,
        const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent)
{
    uint64_t args[2];
    // Set up scalar args, then heap args
    args[ 0] = (uint64_t)(size);
    args[ 1] = (uint64_t)(mat);

    CHECK_HSTR_RESULT(
        hStreams_EnqueueCompute(
            in_LogicalStr,
            "hStreams_custom_dgetrf_square_nopiv_rowMaj_sink",
            1,            // scalar args
            1,             // heap args
            args,          // arg array
            out_pEvent,    // event
            NULL,          // return value pointer
            0));           // return value size

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_dgetrf_square_nopiv_colMaj(int size, double *mat,
        const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent)
{
    uint64_t args[2];
    // Set up scalar args, then heap args
    args[ 0] = (uint64_t)(size);
    args[ 1] = (uint64_t)(mat);

    CHECK_HSTR_RESULT(
        hStreams_EnqueueCompute(
            in_LogicalStr,
            "hStreams_custom_dgetrf_square_nopiv_colMaj_sink",
            1,            // scalar args
            1,             // heap args
            args,          // arg array
            out_pEvent,    // event
            NULL,          // return value pointer
            0));           // return value size

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_dgemm(
    const HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const MKL_INT M, const MKL_INT N, const MKL_INT K, const double alpha,
    const double *A, const MKL_INT lda,
    const double *B, const MKL_INT ldb, const double beta,
    double *C, const MKL_INT ldc, HSTR_EVENT    *out_pEvent)
{
    doubleToUint64_t uAlpha, uBeta;
    uint64_t args[14];

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

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT
hStreams_custom_memset(
    HSTR_LOG_STR in_LogStreamID,
    void *in_Dest, int in_Val, uint64_t in_NumBytes,
    HSTR_EVENT    *out_pEvent)
{
    uint64_t args[3];

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

    return HSTR_RESULT_SUCCESS;
}

void ShowLimitCPUmask(const HSTR_CPU_MASK         in_cpu_mask)
{
    int i, first, last, num_set;

    // Todo: change to something more portable
    last = 0;
    first = HSTR_MAX_THREADS;
    num_set = 0;
    for (i = 0; i < HSTR_MAX_THREADS; i++) {
        if (HSTR_CPU_MASK_ISSET(i, in_cpu_mask)) {
            if (i < first) {
                first = i;
            }
            last = i;
            num_set++;
        }
    }

    printf(
        "CPU set has %d bits set, from %d to %d\n",
        num_set, first, last);
}
