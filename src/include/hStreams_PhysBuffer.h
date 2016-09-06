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

#ifndef HSTREAMS_PHYSBUFFER_H
#define HSTREAMS_PHYSBUFFER_H

#include <vector>

#include "hStreams_RefCountDestroyed.h"
#include "hStreams_types.h"
#include "hStreams_locks.h"
#include "hStreams_COIWrapper.h"

// forward declaration
class hStreams_LogBuffer;

/// @brief An instantiation of a logical buffer for a given logical domain
class hStreams_PhysBuffer : public hStreams_RefCountDestroyed
{
    /// @brief A mutex to synchronize internal operations
    hStreams_Lock lock_;
    /// @brief A vector of actions in which this buffer is involved.
    /// Destruction of the buffer will wait on the completion of these actions.
    std::vector<HSTR_EVENT> pending_actions_;
    /// @brief A number of addPendingAction() calls from last removeDoneActions() call.
    uint64_t action_cleanup_counter_;
    /// @brief A logical buffer with contain this physical buffer
    const hStreams_LogBuffer *log_buf_;
    /// @brief A handle to the COI buffer
    const HSTR_COIBUFFER coi_buf_;
    /// @brief Sink-side buffers are a bit larger to ensure the same offset into the cache
    ///     line.
    const uint64_t padding_;
    /// @brief The sink-side address of the buffer's beginning
    const uint64_t sink_start_addr_;
    /// @brief This class is used in removeCompletedActions() as condition to remove action
    class isActionCompleted_functor_;
public:
    /// @param[in] HSTR_COIBUFFER a handle to a pre-created COI buffer
    /// @param[in] sink_start_addr a pre-looked-up sink-side address of the buffer's beginning
    /// @param[in] padding The amount of superfluous memory at the beginning of the buffer;
    ///     usually, the offset of the host side buffer into the cache line
    hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, uint64_t sink_start_addr, uint64_t padding);
    hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, void *sink_start_addr, uint64_t padding);
    /// @note While this _could_ be provided in LogBuffer only, this allows Enqueue* functions to
    ///     not care whether the buffer is on source or sink, which will be relevant for
    ///     aliased buffers on the host
    uint64_t getPadding() const;
    /// @brief Get the sink-side address of memory location offset by \c host_offset from the
    ///     beginning in the source proxy memory address space.
    uint64_t translateToSinkAddress(uint64_t host_offset) const;
    /// @brief Register an action that concerns this buffer
    /// Already-completed events are removed after a certain number of calls.
    void addPendingAction(HSTR_EVENT action);
    /// @brief Remove completed actions from set of actions which concerns this buffer.
    void removeCompletedActions();
    /// @brief Get a reference to parent log buffer
    const hStreams_LogBuffer &getLogBuffer() const;
    /// @brief Get a copy of COI buffer handle
    HSTR_COIBUFFER getCOIhandle() const;
protected:
    virtual ~hStreams_PhysBuffer();
};

bool operator==(hStreams_PhysBuffer const &pb1, hStreams_PhysBuffer const &pb2);
bool operator!=(hStreams_PhysBuffer const &pb1, hStreams_PhysBuffer const &pb2);

#endif /* HSTREAMS_PHYSBUFFER_H */
