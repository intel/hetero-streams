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

#ifndef __HSTR_APP_API_H__
#define __HSTR_APP_API_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <mkl.h>
#include <stdint.h>
// Other headers contain definitions which might be
// of use to those including hStreams_app_api.h
#include <hStreams_version.h>
#include <hStreams_common.h>
#include <hStreams_source.h>
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////
// Doxygen settings
/// @file include/hStreams_app_api.h
///
/// @defgroup hStreams_AppApi app API (source)
/// The app API is set of simplified functions covering only a subset of hStreams'
/// functionality. It is designed to help an inexperienced hStreams user quickly
/// start developing their own applications using the library.
///
/// Apart from the library's \ref hStreams_AppApi_Core "core functionality", several \ref
/// hStreams_AppApi_Common "common building blocks" are offered to further help
/// new users quickly develop their code.
/// @defgroup hStreams_AppApi_Core Wrapped and simplified core functions
/// @ingroup hStreams_AppApi
/// @defgroup hStreams_AppApi_Common Common building blocks
/// @ingroup hStreams_AppApi
/// These functions are provided as examples of common building blocks for an application
/// making use of the hStreams library. Two memory-related functions are provided -
/// hStreams_app_memset() and hStreams_app_memcpy(). There are also four functions which perform
/// remote matrix multiplication using kernels from the Intel(R) Math Kernel Library (Intel(R) MKL).
/// Their parameters correspond to those used by the Intel(R) MKL routines.
///////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
///
// hStreams_app_init
/// @ingroup hStreams_AppApi_Core
/// @brief Initialize hStreams homogenously across all available Intel(R) Xeon Phi(TM) coprocessors
///
/// If no initialization through \c hStreams_app_init* routines had been
/// performed beforehand, this function will detect all available Intel(R) Xeon
/// Phi(TM) coprocessors and attempt to create one logical domain on each of
/// them. In each of those logical domains, \c in_StreamsPerDomain sets of
/// streams will be created, each of them consisting of \c
/// in_LogStreamOversubscription exactly overlapping logical streams.
///
/// The IDs of the logical domains created will start at \c 1 and end with the
/// number of Intel(R) Xeon Phi(TM) coprocessors available in the system. The
/// IDs of the logical streams created will start at \c 0 and end at \c
/// in_StreamsPerDomain \c * \c in_LogStreamOversubscription \c - \c 1.
///
/// None of the first \c in_StreamsPerDomain streams will overlap each other.
/// If <tt>in_LogStreamOversubscription > 0</tt>, stream number \c in_StreamsPerDomain
/// will exactly overlap stream number \c 0, stream number <tt>in_StreamsPerDomain + 1</tt>
/// will exactly overlap stream number \c 1 and so on.
///
/// For subsequent invocations of \c hStreams_app_init*, those functions will
/// attempt to reuse the logical domains created by the first app API-level
/// initialization and create an additional distribution of logical streams in
/// those logical domains.  The IDs of the newly created logical streams will
/// be enumerated in a similar fashion to what is described above with the
/// exception that the lowest-numbered new logical stream ID will be equal to
/// the highest ID from streams added in previous initializations plus one.
///
/// @param  in_StreamsPerDomain
///         [in] number of physical streams to create in each logical domain
///
/// @param  in_LogStreamOversubscription
///         [in] degree of oversubscription for logical streams
///
/// @return If successful, \c hStreams_app_init() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_DEVICE_NOT_INITIALIZED if hStreams cannot be initialized.
///     Possible causes include incorrect values of environment variables or
///     no Intel(R) Xeon Phi(TM) coprocessor being available
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if this call is \e not the first
///     initialization and the desired number of logical domains does not match
///     what was created during the first initialization.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_LogStreamOversubscription is \c 0
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_StreamsPerDomain is \c 0
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_StreamsPerDomain exceeds the number of
///     logical CPUs available in any of the Intel(R) Xeon Phi(TM) coprocessors detected
/// @arg \c HSTR_RESULT_ALREADY_FOUND if an attempt is made to add a stream that is
///     already present. This can be returned if the initialization has already happened.
/// @arg \c HSTR_RESULT_BAD_NAME if dynamic-link dependences cannot be located
///
/// @thread_safety Not thread safe.
///
//////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_init_in_version(
    uint32_t    in_StreamsPerDomain,
    uint32_t    in_LogStreamOversubscription,
    const char* interface_version);

static
DllAccess HSTR_RESULT
hStreams_app_init(
    uint32_t    in_StreamsPerDomain,
    uint32_t    in_LogStreamOversubscription)
{
    return hStreams_app_init_in_version(in_StreamsPerDomain,
        in_LogStreamOversubscription, HSTR_VERSION_STRING);
}

//////////////////////////////////////////////////////////////////
///
// hStreams_app_init_domains
/// @ingroup hStreams_AppApi_Core
/// @brief Initialize hStreams state, allowing for non-heterogeneity and more control
///         then \c hStreams_app_init()
///
/// If no initialization through \c hStreams_app_init* routines had been
/// performed beforehand, this function will detect all available Intel(R) Xeon
/// and attempt to spread an indicated number of logical domain across all of
/// them. In i-th logical domain, <tt>in_StreamsPerDomain[i]</tt> sets of
/// streams will be created, each of them consisting of \c
/// in_LogStreamOversubscription exactly overlapping logical streams. The
/// different sets of streams will not overlap.
///
/// The IDs of the logical domains created will start at \c 1 and end with \c
/// in_NumLogDomains. The IDs of the logical streams created will start at \c 0
/// and end at <tt>(in_pStreamsPerDomain[0] + ... +
/// in_pStreamsPerDomain[in_NumLogDomains-1]) *
/// in_LogStreamOversubscription</tt>.
///
/// With \c in_LogStreamOversubscription larger than \c 1 logical streams are
/// numbered across all the physical streams first vs within.
/// This causes noncontinuous logical streams IDs inside single physical stream.
///
/// For subsequent invocations of \c hStreams_app_init*, those functions will
/// attempt to reuse the logical domains created by the first app API-level
/// initialization and create an additional distribution of logical streams in
/// those logical domains.  The IDs of the newly created logical streams will
/// be enumerated in a similar fashion to what is described above with the
/// exception that the lowest-numbered new logical stream ID will be equal to
/// the highest ID from streams added in previous initializations plus one.
///
/// If \c in_NumLogDomains is not an even multiple of available Intel(R) Xeon Phi(TM)
/// coprocessors or the coprocessors have different number of hardware threads,
/// the logical domains will not be uniform; using as many hardware threads as
/// possible is favored over uniformity of logical domains.
///
/// @param  in_NumLogDomains
///         [in] number of logical domains to spread across available Intel(R) Xeon Phi(TM) coprocessors
///
/// @param  in_pStreamsPerDomain
///         [in] physical streams per logical domain.
///         If some values are equal to 0, the corresponding domains are unused
///
/// @param  in_LogStreamOversubscription
///         [in] number of logical streams that should map to each of the physical
///         streams created
///
/// @return If successful, \c hStreams_app_init_domains() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_DEVICE_NOT_INITIALIZED if hStreams cannot be initialized.
///     Possible causes include incorrect values of environment variables or
///     no Intel(R) Xeon Phi(TM) coprocessor being available
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pStreamsPerDomain is \c NULL
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_NumLogDomains is \c 0
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if this call is \e not the first
///     initialization and the desired number of logical domains does not match
///     what was created during the first initialization.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_LogStreamOversubscription is \c 0
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if a value of an entry in \c in_pStreamsPerDomain
///     exceeds the number of hardware threads available in that specific domain
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if all entries of \c in_pStreamsPerDomain are \c 0
/// @arg \c HSTR_RESULT_ALREADY_FOUND if an attempt is made to add a stream that is
///     already present. This can be returned if the initialization has already happened.
/// @arg \c HSTR_RESULT_BAD_NAME if dynamic-link dependences cannot be located
///
/// @thread_safety Not thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_init_domains_in_version(
    uint32_t    in_NumLogDomains,
    uint32_t   *in_pStreamsPerDomain,
    uint32_t    in_LogStreamOversubscription,
    const char *interface_version);

static
DllAccess HSTR_RESULT
hStreams_app_init_domains(
    uint32_t    in_NumLogDomains,
    uint32_t   *in_pStreamsPerDomain,
    uint32_t    in_LogStreamOversubscription)
{
    return hStreams_app_init_domains_in_version( in_NumLogDomains, in_pStreamsPerDomain,
            in_LogStreamOversubscription, HSTR_VERSION_STRING);
}

///////////////////////////////////////////////////////////////////
///
// hStreams_app_fini
/// @ingroup hStreams_AppApi_Core
/// @brief Finalization of hStreams state
///
/// Destroys hStreams internal structures and clears the state of the library.
/// All logical domains, streams and buffers are destroyed as a result of this call.
///
/// @return If successful, \c hStreams_app_fini() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
///
/// @thread_safety Thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_fini();

///////////////////////////////////////////////////////////////////
///
// hStreams_app_create_buf
/// @ingroup hStreams_AppApi_Core
/// @brief Allocate 1-dimensional buffer on each currently existing logical domains.
///
/// Construct an hStreams buffer out of user-provided memory. This function creates
/// an instantiation of the buffer in all of the currently present logical domains.
/// Note that the contents of those instatiations are considered to be undefined, i.e.
/// the contents of the buffer are not implicitly synchronized across all the
/// instantations.
///
/// A buffer constructed by a call to \c hStreams_app_create_buf() may be later supplied
/// as an operand to memory transfer or a compute action. In order to do that, user
/// should supply a valid address falling anywhere \em inside the buffer.
///
/// @param  in_BufAddr
///         [in] pointer to the beginning of the memory in the source logical domain
/// @param  in_NumBytes
///         [in] size of the memory to create the buffer for, in bytes
///
/// @return If successful, \c hStreams_app_create_buf() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_BaseAddress is \c NULL
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumBytes == 0</tt>
/// @arg \c HSTR_RESULT_OUT_OF_MEMORY if there's not enough memory on one or
///     more domains and therefore the buffer cannot be instantiatied
///
/// @thread_safety Thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_create_buf(void *in_BufAddr, const uint64_t in_NumBytes);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_xfer_memory
/// @ingroup hStreams_Source_StreamUsage
/// @brief Enqueue 1-dimensional data transfers in a logical stream.
///
/// @note Data transfers are permitted to execute out-of-order subject to dependence
///     policy and the dependences previously inserted into the stream.
/// @sa \c HSTR_OPTIONS.dep_policy
///
/// @note This API allows for memory transfers between the \e source and the \e
///     sink logical domain, not between arbitrary sink logical domain.
/// @sa \c hStreams_EnqueueDataXDomain1D() for an API allowing transfers between
///     arbitrary logical domains
///
/// @param  in_LogStreamID
///         [in] The ID of the logical stream to insert the data transfer action in.
///
/// @param  in_pWriteAddr
///         [in] Source proxy pointer to the memory location to write to
///
/// @param  in_pReadAddr
///         [in] Source proxy pointer to the memory location to read from
///
/// @param  in_size
///         [in] The size, in bytes, of contiguous memory that should be copied
///
/// @param  in_XferDirection
///         [in] The direction in which the memory transfer should occur
///
/// @param  out_pEvent
///         [out] optional, the pointer for the completion event
///
/// @return If successful, \c hStreams_app_xfer_memory() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pWriteAddr is \c NULL
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pReadAddr is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if there is no logical stream with ID equal to
///     \c in_LogStreamID
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pWriteAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pReadAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the data locations involved in the transfer
///     fall outside of any of the buffers' boundaries.
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_xfer_memory() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_xfer_memory(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_NumBytes,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_invoke
/// @ingroup hStreams_AppApi_Core
/// @brief Enqueue an execution of a user-defined function in a stream
///
/// Places an execution of a user-defined function in the stream's internal queue.
/// The function to be called shall be compiled and loaded to the sink process so
/// that hStreams can locate the appropriate symbol and invoke it on the stream's
/// sink endpoint.
///
/// @param  in_LogStreamID
///         [in] ID of logical stream associated to enqueue the action in
///
/// @param  in_pFuncName
///         [in] Null-terminated string with name of the function to be executed
///
/// @param  in_NumScalarArgs
///         [in] Number of arguments to be copied by value for remote invocation
///
/// @param  in_NumHeapArgs
///         [in] Number of arguments which are buffer addreses to be translated
///         to sink-side instantiations' addresses
///
/// @param  in_pArgs
///         [in] Array of in_NumScalarArgs+in_NumHeapArgs arguments as 64-bit unsigned
///         integers with scalar args first and buffer args second
///
/// @param  out_pEvent
///         [out] pointer to event which will be signaled once the action
///         completes
///
/// @param  out_pReturnValue
///         [out] pointer to host-side memory the remote invocation can
///         asynchronously write to
///
/// @param  in_ReturnValueSize
///         [in] the size of the asynchronous return value memory
///
/// @return If successful, \c hStreams_app_invoke() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID is not a valid logical stream ID
/// @arg \c HSTR_RESULT_NOT_FOUND if at least one of the buffer arguments is not
///     in a buffer that had been instantiated for \c in_LogStreamID's logical domain
/// @arg \c HSTR_RESULT_BAD_NAME if \c in_pFunctionName is \c NULL
/// @arg \c HSTR_RESULT_BAD_NAME if no symbol named \c in_pFunctionName is found
///     on the streams's sink endpoint
/// @arg \c HSTR_RESULT_BAD_NAME if \c in_pFunctionName is longer than \c
///     HSTR_MAX_FUNC_NAME_SIZE
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_ReturnValueSize exceeds \c
///     HSTR_RETURN_SIZE_LIMIT
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if <tt>in_ReturnValueSize != 0</tt>
///     and \c in_pReturnValue is \c NULL or <tt>in_ReturnValueSize == 0</tt> and \c
///     in_pReturnValue is not \c NULL
/// @arg \c HSTR_RESULT_TOO_MANY_ARGS if <tt>in_NumScalarArgs +
///     in_NumHeapArgs > HSTR_ARGS_SUPPORTED</tt>
/// @arg \c HSTR_RESULT_NULL_PTR <tt>in_numScalarArgs + in_numHeapArgs > 0</tt> but
///     \c in_pArgs is \c NULL
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_invoke() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_invoke(
    HSTR_LOG_STR   in_LogStreamID,
    const char    *in_pFuncName,
    uint32_t       in_NumScalarArgs,
    uint32_t       in_NumHeapArgs,
    uint64_t      *in_pArgs,
    HSTR_EVENT    *out_pEvent,
    void          *out_pReturnValue,
    uint16_t       in_ReturnValueSize
);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_stream_sync
/// @ingroup hStreams_AppApi_Core
/// @brief Block until all the operation enqueued in a stream have completed
///
/// @param  in_LogStreamID
///         [in] ID of the logical stream
///
/// @return If successful, \c hStreams_app_stream_sync() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID is not a valid logical stream ID
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if timeout was reached while waiting on the
///     completion of stream's actions.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if one of the events in the stream was canceled
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
///
/// @see HSTR_OPTIONS.time_out_ms_val
/// @thread_safety Thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_stream_sync(HSTR_LOG_STR in_LogStreamID);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_thread_sync
/// @ingroup hStreams_AppApi_Core
/// @brief Block until all the operation enqueued in all the streams have completed
///
/// @return If successful, \c hStreams_app_thread_sync() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if timeout was reached while waiting on the
///     completion of actions.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if one of the events was canceled
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
///
/// @see HSTR_OPTIONS.time_out_ms_val
///
/// @thread_safety Thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_thread_sync();

///////////////////////////////////////////////////////////////////
///
// hStreams_app_event_wait
/// @ingroup hStreams_AppApi_Core
/// @brief Wait on a set of events
///
/// Synchronization:
///   - Every action - data transfer or remote compute - yields a sync event
///   - Those sync events can be waited on within a given logical stream,
///     with event_wait and event_wait_in_stream.
///
/// @param  in_NumEvents
///         [in] number of event pointers in the array
///
/// @param  in_pEvents
///         [in] array of pointers of events to be waited on
///
/// @return If successful, \c hSterams_app_event_wait() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns on of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR if in_pEvents is NULL
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if the time out was reached or the timeout is zero
///         and the event has not been signalled.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if the event was cancelled or the process died
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if in_NumEvents == 0
///
/// @thread_safety Thread safe.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_event_wait(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_event_wait_in_stream
/// @ingroup hStreams_AppApi_Core
/// @brief Aggregate multiple dependences into one event handle and optionally insert
///     that event handle into a logical stream
///
/// The resulting aggregation of multiple dependences is composed of depenences on:
/// \arg last actions actions enqueued in the stream which involve buffers specified in the
///     \c in_pAddresses array
/// \arg explicit completion events specified in the in_pEvents array
/// The output event handle, \c out_pEvent will represent that aggregation of dependences.
///
/// The special pair of values <tt>(-1, NULL)</tt> for <tt>(in_NumAddresses,
/// in_pAddresses)</tt> is used to signify that the dependence to be created should not
/// be inserted into the stream, only an event handle representing that dependence should
/// be returned.
///
/// The special pair of values <tt>(0, NULL)</tt> for <tt>(in_NumAddresses,
/// in_pAddresses)</tt> is used to signify that the dependence to be created should not
/// involve any actions related to buffer.
///
/// On the other hand, the special pair of values <tt>(0, NULL)</tt> for <tt>(in_NumEvents,
/// in_pEvents)</tt> is used to signify that the dependence to be inserted should include
/// all the actions previously enqueued in the specified stream.
///
/// @param  in_LogStreamID
///         [in] ID of the logical stream from which take the dependencies and into which
///         to optionally insert the dependence.
///
/// @param  in_NumEvents
///         [in] Number of entries in the \c in_pEvents array.
///
/// @param  in_pEvents
///         [in] An array of event handles that should be included in the aggregated dependence.
///
/// @param  in_NumAddresses
///         [in] Number of entries in the in_pAddresses array.
///
/// @param  in_pAddresses
///         [in] Array of source-side proxy addresses to be mapped to buffers, on which
///          the dependencies will be computed for aggregation.
///
/// @param  out_pEvent
///         [out] the aggreagated completion event. If no handle is needed, set this to NULL
///
/// @return If successful, \c hStreams_app_event_wait_in_stream() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NOT_FOUND if a logical stream with ID \c in_LogStreamID doesn't
///     exist
/// @arg \c HSTR_RESULT_NOT_FOUND if at least one entry in the \c in_pAddresses array
///     does not correspond to a buffer with an instantiation in the logical domain in
///     which the \c in_LogStreamID stream is located is located.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_NumAddresses is 0 or -1 and \c
///     in_pAddresses is not NULL and vice versa.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_NumEvents is 0 and \c
///     in_pEvents is not NULL and vice versa.
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_event_wait_in_stream() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_event_wait_in_stream(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT        *out_pEvent);


///////////////////////////////////////////////////////////////////
///
// hStreams_app_memset
/// @ingroup hStreams_AppApi_Common
/// @brief Set remote memory to a value, using a named stream
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  in_pWriteAddr
///         [in] Host proxy address pointer to the base of a memory area
///         to write in_Value to.
///         This address gets mapped to a corresponding address in
///          the sink domain associated with in_LogStreamID
///
/// @param  in_Value
///         [in] the byte-sized value that memory is set to
///
/// @param  in_NumBytes
///         [in] the number of bytes of the sink buffer to be
///         set or copied
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if in_pWriteAddr is NULL
///
/// @return HSTR_RESULT_NOT_FOUND if in_LogStreamID is not found to
///         have an associated hStream, or at least one of the heap arguments
///         is not in an allocated buffer.
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_memset() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_memset(HSTR_LOG_STR in_LogStreamID,
                    void        *in_pWriteAddr,
                    int          in_Value,
                    uint64_t     in_NumBytes,
                    HSTR_EVENT  *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_memcpy
/// @ingroup hStreams_AppApi_Common
/// @brief copy remote memory, using a named stream
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  in_pWriteAddr
///         [in] Host proxy address pointer to the base of a memory area
///         to write the copy to.
///         This address gets mapped to a corresponding address in
///          the sink domain associated with in_LogStreamID
///
/// @param  in_pReadAddr
///         [in] Host proxy address pointer to the base of a memory area
///         to read the copy from.
///         This address gets mapped to a corresponding address in
///          the sink domain associated with in_LogStreamID
///
/// @param  in_NumBytes
///         [in] the number of bytes of the sink buffer to be
///         set or copied
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if in_pWriteAddr or in_pReadAddr is NULL
///
/// @return HSTR_RESULT_NOT_FOUND if in_LogStreamID is not found to
///         have an associated hStream, or at least one of the heap arguments
///         is not in an allocated buffer.
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_memcpy() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_memcpy(HSTR_LOG_STR in_LogStreamID,
                    void        *in_pWriteAddr,
                    void        *in_pReadAddr,
                    uint64_t     in_NumBytes,
                    HSTR_EVENT  *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_sgemm
/// @ingroup hStreams_AppApi_Common
/// @brief perform a remote cblas sgemm
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  CBLAS-related parameters
///         [in] MKL CBLAS input parameters, in their API order
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if A, B or C is NULL
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_sgemm() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_sgemm(
    HSTR_LOG_STR in_LogStreamID,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const float alpha,
    const float *A, const int64_t ldA,
    const float *B, const int64_t ldB, const float beta,
    float *C, const int64_t ldC, HSTR_EVENT    *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_dgemm
/// @ingroup hStreams_AppApi_Common
/// @brief perform a remote cblas dgemm
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  CBLAS-related parameters
///         [in] MKL CBLAS input parameters, in their API order
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if A, B or C is NULL
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_dgemm() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_dgemm(
    HSTR_LOG_STR in_LogStreamID,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const double alpha,
    const double *A, const int64_t ldA,
    const double *B, const int64_t ldB, const double beta,
    double *C, const int64_t ldC, HSTR_EVENT    *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_cgemm
/// @ingroup hStreams_AppApi_Common
/// @brief perform a remote cblas cgemm
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  CBLAS-related parameters
///         [in] MKL CBLAS input parameters, in their API order
///
/// NOTE: the actual types of A, B, C, alpha and beta are MKL_Complex8 *.
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if A, B or C is NULL
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_cgemm() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
///////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_cgemm(
    HSTR_LOG_STR in_LogStreamID,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t ldA,
    const void *B, const int64_t ldB, const void *beta,
    void *C, const int64_t ldC, HSTR_EVENT    *out_pEvent);

///////////////////////////////////////////////////////////////////
///
// hStreams_app_zgemm
/// @ingroup hStreams_AppApi_Common
/// @brief perform a remote cblas zgemm
///
/// @param  in_LogStreamID
///         [in] 0-based index of logical stream
///
/// @param  CBLAS-related parameters
///         [in] MKL CBLAS input parameters, in their API order
///
/// NOTE: the actual types of A, B, C, alpha and beta are MKL_Complex16 *.
///
/// @param  out_pEvent
///         [out] opaque event handle used for synchronization
///
/// @return HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @return HSTR_RESULT_NULL_PTR if A, B or C is NULL
///
/// @return HSTR_RESULT_SUCCESS if successful
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_app_zgemm() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
////////////////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_app_zgemm(
    HSTR_LOG_STR in_LogStreamID,
    const CBLAS_ORDER Order, const CBLAS_TRANSPOSE TransA, const CBLAS_TRANSPOSE TransB,
    const int64_t M, const int64_t N, const int64_t K, const void *alpha,
    const void *A, const int64_t ldA,
    const void *B, const int64_t ldB, const void *beta,
    void *C, const int64_t ldC, HSTR_EVENT    *out_pEvent);


#ifdef __cplusplus
}
#endif

#endif
