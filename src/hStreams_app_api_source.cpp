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

#include "hStreams_app_api.h"
#include "hStreams_source.h"
#include "hStreams_common.h"
#include "hStreams_internal.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_Logger.h"
#include "hStreams_helpers_common.h"
#include "hStreams_Logger.h"

#include "hStreams_app_api_workers_source.h"


HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_init_domains_in_version)(
    uint32_t     in_NumLogDomains,
    uint32_t    *in_pStreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pStreamsPerDomain);
        HSTR_TRACE_API_ARG(in_LogStreamOversubscription);
        HSTR_TRACE_API_ARG_STR(in_InterfaceVersion);

        detail::app_init_domains_in_version_impl_throw(
            in_NumLogDomains, in_pStreamsPerDomain,
            in_LogStreamOversubscription, in_InterfaceVersion);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

/**
 * Corresponds to \c hStreams_app_init_domains() external symbol exposed pre-1.0.0
 *
 * Note that there is no \c hStreams_app_init_domains()  symbol anymore as \c
 * hStreams_app_init_domains() is now an inline function in the header.
 */
HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_init_domains,
        HSTREAMS_1.0)(
    uint32_t     in_NumLogDomains,
    uint32_t    *in_pStreamsPerDomain,
    uint32_t     in_LogStreamOversubscription)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pStreamsPerDomain);
        HSTR_TRACE_API_ARG(in_LogStreamOversubscription);

        detail::app_init_domains_in_version_impl_throw(
            in_NumLogDomains, in_pStreamsPerDomain,
            in_LogStreamOversubscription, HSTR_MAGIC_PRE1_0_0_VERSION_STRING);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

/**
 * Implements the external symbol hStreams_app_init_in_version
 */
HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_init_in_version)(
    uint32_t     in_StreamsPerDomain,
    uint32_t     in_LogStreamOversubscription,
    const char  *in_InterfaceVersion)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_StreamsPerDomain);
        HSTR_TRACE_API_ARG(in_LogStreamOversubscription);
        HSTR_TRACE_API_ARG_STR(in_InterfaceVersion);

        detail::app_init_in_version_impl_throw(
            in_StreamsPerDomain, in_LogStreamOversubscription,in_InterfaceVersion);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

/**
 * Corresponds to \c hStreams_app_init() external symbol exposed pre-1.0.0
 *
 * Note that there is no \c hStreams_app_init()  symbol anymore as \c
 * hStreams_app_init() is now an inline function in the header.
 */
HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_init,
        HSTREAMS_1.0)(
    uint32_t     in_StreamsPerDomain,
    uint32_t     in_LogStreamOversubscription)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_StreamsPerDomain);
        HSTR_TRACE_API_ARG(in_LogStreamOversubscription);

        detail::app_init_in_version_impl_throw(
            in_StreamsPerDomain, in_LogStreamOversubscription,
            HSTR_MAGIC_PRE1_0_0_VERSION_STRING);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_fini,
        HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_RETURN(hStreams_Fini());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_fini)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_RETURN(hStreams_Fini());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_create_buf,
        HSTREAMS_1.0)(
    void *in_BufAddr,
    const uint64_t in_NumBytes)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BufAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);

        HSTR_RETURN(hStreams_Alloc1D(
                in_BufAddr,
                in_NumBytes
            ));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_create_buf)(
    void *in_BufAddr,
    const uint64_t in_NumBytes)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BufAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);

        HSTR_RETURN(hStreams_Alloc1D(
                in_BufAddr,
                in_NumBytes
            ));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_xfer_memory,
        HSTREAMS_1.0)(
    void               *in_pReadAddr,
    void               *in_pWriteAddr,
    uint64_t            in_NumBytes,
    HSTR_LOG_STR        in_LogStreamID,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_XferDirection);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(
            hStreams_EnqueueData1D(
                in_LogStreamID,    // logical stream
                in_pWriteAddr,     // address in dest domain
                in_pReadAddr,      // address in source domain
                in_NumBytes,       // size
                in_XferDirection,  // xfer direction
                out_pEvent         // completion event
            ));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_xfer_memory,
        HSTREAMS_2.0)(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_NumBytes,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(in_XferDirection);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(
            hStreams_EnqueueData1D(
                in_LogStreamID,    // logical stream
                in_pWriteAddr,     // address in dest domain
                in_pReadAddr,      // address in dest domain
                in_NumBytes,       // size
                in_XferDirection,  // xfer direction
                out_pEvent         // completion event
            ));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_xfer_memory)(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_NumBytes,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(in_XferDirection);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(
            hStreams_EnqueueData1D(
                in_LogStreamID,    // logical stream
                in_pWriteAddr,     // address in dest domain
                in_pReadAddr,      // address in dest domain
                in_NumBytes,       // size
                in_XferDirection,  // xfer direction
                out_pEvent         // completion event
            ));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_invoke,
        HSTREAMS_1.0)(
    HSTR_LOG_STR   in_LogStreamID,
    const char    *in_pFuncName,
    uint32_t       in_NumScalarArgs,
    uint32_t       in_NumHeapArgs,
    uint64_t      *in_pArgs,
    void          *out_pReturnValue,
    uint32_t       in_ReturnValueSize,
    HSTR_EVENT    *out_pEvent
)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFuncName);
        HSTR_TRACE_API_ARG(in_NumScalarArgs);
        HSTR_TRACE_API_ARG(in_NumHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                in_pFuncName,
                in_NumScalarArgs,
                in_NumHeapArgs,
                in_pArgs,
                out_pEvent,
                out_pReturnValue,
                in_ReturnValueSize));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_invoke,
        HSTREAMS_2.0)(
    HSTR_LOG_STR   in_LogStreamID,
    const char    *in_pFuncName,
    uint32_t       in_NumScalarArgs,
    uint32_t       in_NumHeapArgs,
    uint64_t      *in_pArgs,
    HSTR_EVENT    *out_pEvent,
    void          *out_pReturnValue,
    uint16_t       in_ReturnValueSize
)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFuncName);
        HSTR_TRACE_API_ARG(in_NumScalarArgs);
        HSTR_TRACE_API_ARG(in_NumHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_TRACE_API_ARG(out_pReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);

        HSTR_RETURN(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                in_pFuncName,
                in_NumScalarArgs,
                in_NumHeapArgs,
                in_pArgs,
                out_pEvent,
                out_pReturnValue,
                in_ReturnValueSize));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_invoke)(
    HSTR_LOG_STR   in_LogStreamID,
    const char    *in_pFuncName,
    uint32_t       in_NumScalarArgs,
    uint32_t       in_NumHeapArgs,
    uint64_t      *in_pArgs,
    HSTR_EVENT    *out_pEvent,
    void          *out_pReturnValue,
    uint16_t       in_ReturnValueSize
)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFuncName);
        HSTR_TRACE_API_ARG(in_NumScalarArgs);
        HSTR_TRACE_API_ARG(in_NumHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_TRACE_API_ARG(out_pReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);

        HSTR_RETURN(
            hStreams_EnqueueCompute(
                in_LogStreamID,
                in_pFuncName,
                in_NumScalarArgs,
                in_NumHeapArgs,
                in_pArgs,
                out_pEvent,
                out_pReturnValue,
                in_ReturnValueSize));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_stream_sync,
        HSTREAMS_1.0)
(HSTR_LOG_STR in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_RETURN(hStreams_StreamSynchronize(in_LogStreamID));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_stream_sync)
(HSTR_LOG_STR in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_RETURN(hStreams_StreamSynchronize(in_LogStreamID));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_thread_sync,
        HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_RETURN(hStreams_ThreadSynchronize());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_thread_sync)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_RETURN(hStreams_ThreadSynchronize());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_event_wait,
        HSTREAMS_1.0)(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        detail::app_event_wait_impl_throw(in_NumEvents, in_pEvents);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_event_wait)(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        detail::app_event_wait_impl_throw(in_NumEvents, in_pEvents);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_event_wait_in_stream,
        HSTREAMS_1.0)(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_NumAddresses);
        HSTR_TRACE_API_ARG(in_pAddresses);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(hStreams_EventStreamWait(in_LogStreamID,
            in_NumEvents,
            in_pEvents,
            in_NumAddresses,
            in_pAddresses,
            out_pEvent));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_event_wait_in_stream)(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_NumAddresses);
        HSTR_TRACE_API_ARG(in_pAddresses);
        HSTR_TRACE_API_ARG(out_pEvent);

        HSTR_RETURN(hStreams_EventStreamWait(in_LogStreamID,
            in_NumEvents,
            in_pEvents,
            in_NumAddresses,
            in_pAddresses,
            out_pEvent));
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_memset,
        HSTREAMS_1.0)(
    void            *in_Dest,
    int              in_Val,
    uint64_t         in_NumBytes,
    HSTR_LOG_STR     in_LogStreamID,
    HSTR_EVENT      *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Dest);
        HSTR_TRACE_API_ARG(in_Val);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memset_impl_throw(in_LogStreamID, in_Dest, in_Val, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_memset,
        HSTREAMS_2.0)(
    HSTR_LOG_STR     in_LogStreamID,
    void            *in_pWriteAddr,
    int              in_Val,
    uint64_t         in_NumBytes,
    HSTR_EVENT      *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_Val);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memset_impl_throw(in_LogStreamID, in_pWriteAddr, in_Val, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_memset)(
    HSTR_LOG_STR     in_LogStreamID,
    void            *in_pWriteAddr,
    int              in_Val,
    uint64_t         in_NumBytes,
    HSTR_EVENT      *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_Val);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memset_impl_throw(in_LogStreamID, in_pWriteAddr, in_Val, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_memcpy,
        HSTREAMS_1.0)(
    void        *in_Src,
    void        *in_Dest,
    uint64_t     in_NumBytes,
    HSTR_LOG_STR in_LogStreamID,
    HSTR_EVENT  *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Src);
        HSTR_TRACE_API_ARG(in_Dest);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memcpy_impl_throw(in_LogStreamID, in_Src, in_Dest, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_memcpy,
        HSTREAMS_2.0)(
    HSTR_LOG_STR in_LogStreamID,
    void        *in_pReadAddr,
    void        *in_pWriteAddr,
    uint64_t     in_NumBytes,
    HSTR_EVENT  *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memcpy_impl_throw(in_LogStreamID, in_pReadAddr, in_pWriteAddr, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_memcpy)(
    HSTR_LOG_STR in_LogStreamID,
    void        *in_pReadAddr,
    void        *in_pWriteAddr,
    uint64_t     in_NumBytes,
    HSTR_EVENT  *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_NumBytes);
        HSTR_TRACE_API_ARG(out_pEvent);
        detail::app_memcpy_impl_throw(in_LogStreamID, in_pReadAddr, in_pWriteAddr, in_NumBytes,
                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_sgemm,
        HSTREAMS_1.0)(
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const float alpha,
    const float *A, const int32_t lda,
    const float *B, const int32_t ldb, const float beta,
    float *C, const int32_t ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_sgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_sgemm,
        HSTREAMS_2.0)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const float alpha,
    const float *A, const int32_t lda,
    const float *B, const int32_t ldb, const float beta,
    float *C, const int32_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_sgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_sgemm)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const float alpha,
    const float *A, const int64_t lda,
    const float *B, const int64_t ldb, const float beta,
    float *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_sgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_dgemm,
        HSTREAMS_1.0)(
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const double alpha,
    const double *A, const int32_t lda,
    const double *B, const int32_t ldb, const double beta,
    double *C, const int32_t ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_dgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_dgemm,
        HSTREAMS_2.0)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const double alpha,
    const double *A, const int32_t lda,
    const double *B, const int32_t ldb, const double beta,
    double *C, const int32_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_dgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_dgemm)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const double alpha,
    const double *A, const int64_t lda,
    const double *B, const int64_t ldb, const double beta,
    double *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_dgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_cgemm,
        HSTREAMS_1.0)(
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const void *alpha,
    const void *A, const int32_t lda,
    const void *B, const int32_t ldb, const void *beta,
    void *C, const int32_t ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_cgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_cgemm,
        HSTREAMS_2.0)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const void *alpha,
    const void *A, const int32_t lda,
    const void *B, const int32_t ldb, const void *beta,
    void *C, const int32_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_cgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_cgemm)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t lda,
    const void *B, const int64_t ldb, const void *beta,
    void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_cgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_zgemm,
        HSTREAMS_1.0)(
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const void *alpha,
    const void *A, const int32_t lda,
    const void *B, const int32_t ldb, const void *beta,
    void *C, const int32_t ldc, const HSTR_LOG_STR LogStream, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_zgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
        HSTR_RESULT,
        hStreams_app_zgemm,
        HSTREAMS_2.0)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int32_t M, const int32_t N, const int32_t K, const void *alpha,
    const void *A, const int32_t lda,
    const void *B, const int32_t ldb, const void *beta,
    void *C, const int32_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_zgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
        HSTR_RESULT,
        hStreams_app_zgemm)(
    HSTR_LOG_STR LogStream,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t lda,
    const void *B, const int64_t ldb, const void *beta,
    void *C, const int64_t ldc, HSTR_EVENT    *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(LogStream);
        HSTR_TRACE_API_ARG(Order);
        HSTR_TRACE_API_ARG(TransA);
        HSTR_TRACE_API_ARG(TransB);
        HSTR_TRACE_API_ARG(M);
        HSTR_TRACE_API_ARG(N);
        HSTR_TRACE_API_ARG(K);
        HSTR_TRACE_API_ARG(alpha);
        HSTR_TRACE_API_ARG(A);
        HSTR_TRACE_API_ARG(lda);
        HSTR_TRACE_API_ARG(B);
        HSTR_TRACE_API_ARG(ldb);
        HSTR_TRACE_API_ARG(beta);
        HSTR_TRACE_API_ARG(C);
        HSTR_TRACE_API_ARG(ldc);
        HSTR_TRACE_API_ARG(out_pEvent);

        detail::app_zgemm_impl_throw(
                   LogStream,
                   Order, TransA, TransB,
                   M, N, K, alpha,
                   A, lda,
                   B, ldb, beta,
                   C, ldc, out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}
