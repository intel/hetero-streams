/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#include <mkl.h>
#include <hStreams_common.h>

// The ConvertToUint64_t template is used to allow data to safely get passed the calling conventions
// in the hstreams API by marshalling/demarshalling data from/to source/sink execution environments.
// For an example of its use, see ref_code/common/hStreams_custom.cpp and
// ref_code/common/hStreams_custom_kernels_sink.cpp
//template<class TYPE, const size_t sz>
//class ConvertToUint64_t
//{
//public:
//
//    // We abort compilation if the expression is not true, which is that the size of the TYPE parameter is less than
//    // or equal to the sz parameter timessizeof(uint64_t).
//    //HSTR_STATIC_ASSERT(sizeof(TYPE) <= sz *sizeof(uint64_t), sizeof_TYPE_must_be_less_than_or_equal_to_sz_times_sizeof_uint64_t);
//
//    typedef uint64_t conversion_t[sz];
//
//    ConvertToUint64_t()
//    {
//        memset(this, 0, sizeof(*this));
//    }
//
//    void Set(const TYPE &t)
//    {
//        mUnion.t = t;
//    }
//
//    void Set_uint64_t(uint64_t u64, size_t idx = 0)
//    {
//        mUnion.c[idx] = u64;
//    }
//
//    TYPE &Get(void)
//    {
//        return mUnion.t;
//    }
//
//    uint64_t Get_uint64_t(size_t idx = 0)
//    {
//        return mUnion.c[idx];
//    }
//
//private:
//    union {
//        TYPE         t;
//        conversion_t c;
//    } mUnion;
//};

//typedef ConvertToUint64_t<char, 1>           charToUint64_t;
//typedef ConvertToUint64_t<float, 1>          floatToUint64_t;
//typedef ConvertToUint64_t<double, 1>         doubleToUint64_t;

HSTR_RESULT
hStreams_custom_dtrsm(const CBLAS_ORDER Order, const CBLAS_SIDE Side,
                      const CBLAS_UPLO Uplo, const CBLAS_TRANSPOSE TransA,
                      const CBLAS_DIAG Diag, const MKL_INT M, const MKL_INT N,
                      const double alpha, const double *A, const MKL_INT lda,
                      double *B, const MKL_INT ldb, const uint32_t in_LogicalStr,
                      HSTR_EVENT *out_pEvent);

HSTR_RESULT
hStreams_custom_dpotrf(int matrix_order, char uplo, lapack_int n,
                       double *a, lapack_int lda, const uint32_t in_LogicalStr,
                       HSTR_EVENT *out_pEvent);

HSTR_RESULT
hStreams_custom_dsyrk(const CBLAS_ORDER Order, const CBLAS_UPLO Uplo,
                      const CBLAS_TRANSPOSE Trans, const MKL_INT N, const MKL_INT K,
                      const double alpha, const double *A, const MKL_INT lda,
                      const double beta, double *C, const MKL_INT ldc,
                      const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent);

HSTR_RESULT
hStreams_custom_dgetrf_square_nopiv_rowMaj(int size, double *mat,
        const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent);

HSTR_RESULT
hStreams_custom_dgetrf_square_nopiv_colMaj(int size, double *mat,
        const uint32_t in_LogicalStr, HSTR_EVENT *out_pEvent);

HSTR_RESULT
hStreams_custom_dgemm(
    const HSTR_LOG_STR in_LogStreamID,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const MKL_INT M, const MKL_INT N, const MKL_INT K, const double alpha,
    const double *A, const MKL_INT ldA,
    const double *B, const MKL_INT ldB, const double beta,
    double *C, const MKL_INT ldC, HSTR_EVENT    *out_pEvent);

HSTR_RESULT
hStreams_custom_memset(HSTR_LOG_STR in_LogStreamID,
                       void *in_Dest, int in_Val, uint64_t in_NumBytes,
                       HSTR_EVENT    *out_pEvent);

void ShowLimitCPUmask(const HSTR_CPU_MASK mask);
