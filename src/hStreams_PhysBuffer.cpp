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

#include "hStreams_PhysBuffer.h"
#include "hStreams_internal.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_Logger.h"
#include "hStreams_LogBuffer.h"

#include <algorithm>

uint64_t hStreams_PhysBuffer::getPadding() const
{
    return padding_;
}

uint64_t hStreams_PhysBuffer::translateToSinkAddress(uint64_t host_offset) const
{
    return sink_start_addr_ + padding_ + host_offset;
}

hStreams_PhysBuffer::hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, uint64_t sink_start_addr, uint64_t padding)
    : log_buf_(&log_buf), coi_buf_(coi_buf), padding_(padding), sink_start_addr_(sink_start_addr), action_cleanup_counter_(0)
{

}

hStreams_PhysBuffer::hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, void *sink_start_addr, uint64_t padding)
    : log_buf_(&log_buf), coi_buf_(coi_buf), padding_(padding), sink_start_addr_((uint64_t)sink_start_addr), action_cleanup_counter_(0)
{

}

hStreams_PhysBuffer::~hStreams_PhysBuffer()
{
    HSTR_COIRESULT coi_res;
    if (!pending_actions_.empty()) {
        coi_res = hStreams_COIWrapper::COIEventWait((uint16_t) pending_actions_.size(), &pending_actions_[0], -1, true, NULL, NULL);
        if (HSTR_COI_SUCCESS != coi_res) {
            HSTR_WARN(HSTR_INFO_TYPE_SYNC)
                    << "Couldn't perform wait for pending actions while destroying buffer: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);
        }
    }

    coi_res = hStreams_COIWrapper::COIBufferDestroy(coi_buf_);
    if (HSTR_COI_SUCCESS != coi_res) {
        HSTR_WARN(HSTR_INFO_TYPE_MEM)
                << "Couldn't destroy buffer [" << log_buf_->getStart() << "]: "
                << hStreams_COIWrapper::COIResultGetName(coi_res);

    }
}

const hStreams_LogBuffer &hStreams_PhysBuffer::getLogBuffer() const
{
    return *log_buf_;
}

HSTR_COIBUFFER hStreams_PhysBuffer::getCOIhandle() const
{
    return coi_buf_;
}

class hStreams_PhysBuffer::isActionCompleted_functor_
{
public:
    isActionCompleted_functor_(uint32_t &index, uint32_t num_completed_actions, uint32_t *completed_actions) :
        index_(index), num_completed_actions_(num_completed_actions), completed_actions_(completed_actions) {}

    bool operator()(const HSTR_EVENT &event)
    {
        for (uint32_t i = 0; i < num_completed_actions_; ++i) {
            if (index_ == completed_actions_[i]) {
                index_++;
                return true;
            }
        }

        index_++;
        return false;
    }
private:
    uint32_t &index_;
    uint32_t num_completed_actions_;
    uint32_t *completed_actions_;
};

void hStreams_PhysBuffer::removeCompletedActions()
{
    // Poll COI which actions are completed (no waiting is done here, timeout is 0)
    HSTR_COIRESULT coi_res;
    uint32_t num_completed_actions = 0;
    uint32_t *completed_actions = new uint32_t[pending_actions_.size()];

    coi_res = hStreams_COIWrapper::COIEventWait((uint16_t) pending_actions_.size(), &pending_actions_[0],
              0, true, &num_completed_actions, &completed_actions[0]);

    if (HSTR_COI_SUCCESS == coi_res) {
        // All actions in pending_actions_ are completed
        pending_actions_.clear();
    } else if (HSTR_COI_TIME_OUT_REACHED == coi_res) {
        // Remove completed actions
        // TODO: Better solution would be keeping the index only in the functor object and use this object
        // with std::reference_wrapper, but this is not currently work with build system.
        uint32_t index = 0;
        isActionCompleted_functor_ is_completed(index, num_completed_actions, completed_actions);

        pending_actions_.erase(
            remove_if(pending_actions_.begin(), pending_actions_.end(), is_completed), pending_actions_.end());
    } else {
        HSTR_WARN(HSTR_INFO_TYPE_SYNC)
                << "Couldn't perform poll for pending actions while looking for done actions: "
                << hStreams_COIWrapper::COIResultGetName(coi_res);
    }

    delete[] completed_actions;
}

void hStreams_PhysBuffer::addPendingAction(HSTR_EVENT action)
{
    hStreams_Scope_Locker_Unlocker autolock(lock_);

    //Lazy pending actions clean up
    if (++action_cleanup_counter_ >= fixed_buffer_actions_cleanup_value) {
        action_cleanup_counter_ = 0;
        removeCompletedActions();
    }

    pending_actions_.push_back(action);
}

bool operator==(hStreams_PhysBuffer const &pb1, hStreams_PhysBuffer const &pb2)
{
    return pb1.getCOIhandle() == pb2.getCOIhandle();
}

bool operator!=(hStreams_PhysBuffer const &pb1, hStreams_PhysBuffer const &pb2)
{
    return !(pb1 == pb2);
}
