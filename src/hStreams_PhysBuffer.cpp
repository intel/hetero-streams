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
#include "hStreams_Logger.h"
#include "hStreams_LogBuffer.h"

uint64_t hStreams_PhysBuffer::getPadding() const
{
    return padding_;
}

uint64_t hStreams_PhysBuffer::translateToSinkAddress(uint64_t host_offset) const
{
    return sink_start_addr_ + padding_ + host_offset;
}

hStreams_PhysBuffer::hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, uint64_t sink_start_addr, uint64_t padding)
    : log_buf_(&log_buf), coi_buf_(coi_buf), padding_(padding), sink_start_addr_(sink_start_addr)
{

}

hStreams_PhysBuffer::hStreams_PhysBuffer(const hStreams_LogBuffer &log_buf, HSTR_COIBUFFER coi_buf, void *sink_start_addr, uint64_t padding)
    : log_buf_(&log_buf), coi_buf_(coi_buf), padding_(padding), sink_start_addr_((uint64_t)sink_start_addr)
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

void hStreams_PhysBuffer::addPendingAction(HSTR_EVENT action)
{
    hStreams_Scope_Locker_Unlocker autolock(lock_);
    if (!pending_actions_.empty()) {
        // future: verify events already present in the vector (polling
        // COIEventWait), drop those i.e. do a cleanup of the already completed
        // events (remove_if/erase)
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
