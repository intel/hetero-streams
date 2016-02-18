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

#ifndef HSTREAMS_APP_API_WORKERS_SOURCE_H
#define HSTREAMS_APP_API_WORKERS_SOURCE_H

#include "hStreams_types.h"
#include <mkl.h> // for CBLAS_TRANSPOSE et al

/**
 * All workers reside in the detail namespace.
 *
 * Ideally we'd want the workers to be void functions
 * which throw on error so that the API function would look like this:
 *
 *      extern "C"
 *      HSTR_RESULT
 *      hStreams_my_function(<the arguments>)
 *      {
 *          try {
 *              HSTR_TRACE_API_ENTER();
 *              HSTR_TRACE_API_ARG(<first arguments>);
 *
 *              detail::my_function(<the arguments>);
 *              HSTR_RETURN(HSTR_RESULT_SUCCESS);
 *          } catch (...) {
 *              HSTR_RETURN(hStreams_handle_exception());
 *          }
 *      }
 *
 */

namespace detail
{

void
app_init_domains_in_version_impl_throw(
    uint32_t     in_NumLogDomains,
    uint32_t    *in_pStreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion);

void
app_init_in_version_impl_throw(
    uint32_t     in_StreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion);

void
app_event_wait_impl_throw(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents);

void
app_memset_impl_throw(
    HSTR_LOG_STR   in_LogStreamID,
    void          *in_pWriteAddr,
    int            in_Val,
    uint64_t       in_NumBytes,
    HSTR_EVENT    *out_pEvent);

void
app_memcpy_impl_throw(
    HSTR_LOG_STR     in_LogStreamID,
    void            *in_pReadAddr,
    void            *in_pWriteAddr,
    uint64_t         in_NumBytes,
    HSTR_EVENT      *out_pEvent);

void
app_sgemm_impl_throw(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const float alpha,
    const float *A, const int64_t lda,
    const float *B, const int64_t ldb, const float beta,
    float *C, const int64_t ldc, HSTR_EVENT    *out_pEvent);

void
app_dgemm_impl_throw(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const double alpha,
    const double *A, const int64_t lda,
    const double *B, const int64_t ldb, const double beta,
    double *C, const int64_t ldc, HSTR_EVENT    *out_pEvent);

void
app_cgemm_impl_throw(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t lda,
    const void *B, const int64_t ldb, const void *beta,
    void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent);

void
app_zgemm_impl_throw(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t lda,
    const void *B, const int64_t ldb, const void *beta,
    void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent);


} // namespace detail



#endif /* HSTREAMS_APP_API_WORKERS_SOURCE_H */
