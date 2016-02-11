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

#ifndef HSTREAMS_PHYSSTREAM_H
#define HSTREAMS_PHYSSTREAM_H

#include <vector>
#include <map>

#include "hStreams_RefCountDestroyed.h"
#include "hStreams_PhysBuffer.h"
#include "hStreams_types.h"
#include "hStreams_internal.h"
#include "hStreams_helpers_source.h"

class hStreams_LogDomain;

/// @brief Abstraction of a FIFO queue. Implementations - COIPipeline and CrossCommPipeline
/// @note This is an abstract class (note the pure virtual methods). Hence, it is never
///     instantiated directly. Rather, the classes which inherit from this one may be
///     instantiated. This class however serves as the common interface to physical streams.
/// @todo This class needs a "insert an event wait in the stream" public method, to allow
///     putting the implementation of \c hStreams_EventStreamWait() inside this
///     class, making it internally synchronized and thus allowing for more
///     concurrency in the use of the API.
/// @todo This class needs a "wait for whatever is enqueued" public method, to allow
///     putting the implementation of \c hStreams_StreamSynchronize() inside this class,
///     making it internally synchronized and thus allowing for more concurrency in the
///     use of the API. One should also create a \c friend-ed path through the
///     \c hStreams_LogStream class for \c hStreams_LogStreamCollection to
///     query all the input dependences to be able to expose a "synchronize all
///     streams" public method which would allow for more encapsulation in
///     the \c hStreams_PhysStream interface.
class hStreams_PhysStream : public hStreams_RefCountDestroyed
{
    /// \brief The cpu mask assigned to this physical stream.
    const hStreams_CPUMask cpu_mask_;
public:
    hStreams_PhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask);
    ~hStreams_PhysStream();

    /// @brief Main entry point for enqueueing "compute" actions in a stream
    /// @note buffer offsets are the offsets the user specified
    ///     (i.e. in_pArgs[i] - buffer_start_on_host. They need not include
    ///     eventual buffer padding for the sinks.
    /// @note \c scalar_args, \c buffer_args, \c buffer_offsets, \c ret_val and
    ///     \c ret_val_size are not validated in any way -- are presumed to be valid.
    /// @note \c func_name is looked up in the physical domain's cache. If it's not found,
    ///     a sink-side lookup using \c hStreams_PhysStream::impl_fetchSinkFunctionAddress()
    ///     is performed and then the physical domain's cache is augmented. These two
    ///     operations (cache lookup and augmentation) are not synchronized together so
    ///     there may be a "race" in that two physical streams request a specific function
    ///     address, upon failure both perform a sink-side lookup and then both write
    ///     the result to the physical domain's cache, one overwriting the other's result.
    ///     It is however expected that both of these sink-side lookups will return the
    ///     same value so it is not deemed a problem. The consistency of the cache object
    ///     is guaranteed by internal synchronisation of accesses (reads or writes) by
    ///     \c hStreams_PhysDomain::getSinkAddress() and
    ///     \c hStreams_PhysDomain::setSinkAddress() implementations.
    HSTR_RESULT enqueueFunction(
        std::string func_name,
        std::vector<uint64_t> &scalar_args,
        std::vector<hStreams_PhysBuffer *> &buffer_args,
        std::vector<uint64_t> &buffer_offsets,
        void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
    );

    /// @note Source and destination offsets are as requested from the API, not
    ///     including eventual buffer padding.
    HSTR_RESULT enqueueTransfer(
        hStreams_PhysBuffer &dst_buf,
        hStreams_PhysBuffer &src_buf,
        uint64_t dst_offset,
        uint64_t src_offset,
        uint64_t length,
        HSTR_EVENT *ret_event
    );

    /// @brief Get all the events which have been created in this stream.
    /// @param[out] the vector to write the events to. It is cleared by the implementation.
    ///
    /// Pretty much equivalent to \c hStreams_PhysStream::getInputDeps() with a
    /// \c IS_BARRIER.
    void getAllEvents(std::vector<HSTR_EVENT> &events);

    /// @brief Get the events that refer to the latest actions for given buffers
    /// @param[in] dep_type The type of dependency (transfer/compute/barrier)
    /// @param[in] buffers  A vector of buffers for which to look up the events
    /// @param[out] deps    The vector of events pertaining to the specified buffers
    void getInputDeps(
        DEP_TYPE dep_type,
        std::vector<hStreams_PhysBuffer *> &buffers,
        std::vector<HSTR_EVENT> &deps);

    /// @brief Create an input dependency for future actions
    /// @param[in] dep_type The type of dependency (transfer/compute/barrier)
    /// @param[in] buffers  A vector of buffers for which to set the input dependency
    ///     for future actions (i.e. an output dependency)
    /// @param[in] completion The event which constitutes the dependency
    ///
    /// @note if dep_type == IS_BARRIER, caller is responsible for providing
    ///     a comprehensive list of buffers in the buffers vector
    ///
    /// @note Say there are some actions enqueued for buffers A and B.
    ///      We then insert a barrier, rightfully providing a full list
    ///      of buffers, e.g. A, B and C. If buffer D is then added
    ///      (through Alloc1D) and an operation is enqueued for that
    ///      buffer only, that operation won't wait until the barrier
    ///      which was inserted before completes.
    ///      We might need to address this shortcoming in the future.
    void setOutputDeps(
        DEP_TYPE dep_type,
        std::vector<hStreams_PhysBuffer *> &buffers,
        HSTR_EVENT completion);

    /// @brief Retrieve a copy of the CPU mask the stream has been created with.
    hStreams_CPUMask getCPUMask() const;

protected:
    /// @brief We save a "link" to the logical domain this stream is contained in.
    hStreams_LogDomain *log_dom_;

private:
    /// @brief A mutex for synchronizing enqueues and waits
    hStreams_Lock lock_;

    /// @brief Dependence tracking meat
    std::map<hStreams_PhysBuffer *, HSTR_EVENT> pendingBufUpdates_;
    /// @brief Dependence tracking meat
    HSTR_EVENT lastAction_;


    /// @brief Interface for the implementation of "enqueue a compute action" functionality
    ///
    /// @note The arguments are tailored to what's used of
    ///     COIPipelineRunFunction (specifically, to what the hStreams thunk
    ///     function expects in its misc data)
    /// @note Implementation of a physical stream must provide an implementation for
    ///     this virtual method in order to be functional.
    virtual HSTR_RESULT impl_enqueueFunction(
        std::vector<uint64_t> &args,
        std::vector<HSTR_EVENT> &input_deps,
        void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
    ) = 0;
};


#endif /* HSTREAMS_PHYSSTREAM_H */
