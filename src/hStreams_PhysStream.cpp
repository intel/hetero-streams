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

#include "hStreams_LogDomain.h"
#include "hStreams_PhysStream.h"
#include "hStreams_PhysDomain.h"
#include "hStreams_LogBuffer.h"
#include "hStreams_PhysBuffer.h"
#include "hStreams_helpers_source.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_Logger.h"

#include <vector>

hStreams_PhysStream::hStreams_PhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask)
    : log_dom_(&log_dom), cpu_mask_(cpu_mask)
{
    lastAction_.opaque[0] = (uint64_t)-1;
    lastAction_.opaque[1] = (uint64_t)-1;
}

hStreams_PhysStream::~hStreams_PhysStream()
{
    std::vector<HSTR_EVENT> pending_actions;
    std::vector<hStreams_PhysBuffer *> dummy_buffers;
    getInputDeps(IS_BARRIER, dummy_buffers, pending_actions);
    if (!pending_actions.empty()) {
        HSTR_COIRESULT coi_res = hStreams_COIWrapper::COIEventWait((uint16_t) pending_actions.size(), &pending_actions[0], -1, true, NULL, NULL);
        if (HSTR_COI_SUCCESS != coi_res) {
            HSTR_WARN(HSTR_INFO_TYPE_SYNC)
                    << "Couldn't perform wait for pending actions while destroying stream: "
                    << hStreams_COIWrapper::COIResultGetName(coi_res);
        }
    }
    log_dom_->getPhysDomain().processPhysStreamDestroy(*this);
}

hStreams_CPUMask hStreams_PhysStream::getCPUMask() const
{
    return cpu_mask_;
}

void hStreams_PhysStream::getAllEvents(std::vector<HSTR_EVENT> &events)
{
    events.clear();
    std::vector<hStreams_PhysBuffer *> dummy_bufs;
    getInputDeps(IS_BARRIER, dummy_bufs, events);
}

void hStreams_PhysStream::getInputDeps(
    DEP_TYPE dep_type,
    std::vector<hStreams_PhysBuffer *> &buffers,
    std::vector<HSTR_EVENT> &deps)
{

    deps.clear(); // don't depend on caller to clear this

    if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_CONSERVATIVE) {
        // dep_type is a don't care
        deps.push_back(lastAction_);
    } else if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_BUFFERS) {
        // FIXME perf: this can be commented out unless and until computes are
        // OOO [prove this first]
        if (dep_type != IS_XFER) {
            deps.push_back(lastAction_);
        }

        if (dep_type == IS_BARRIER) {
            // If barrier, get completion event for every buffer used thus far in the stream
            deps.reserve(deps.size() + pendingBufUpdates_.size());
            std::map<hStreams_PhysBuffer *, HSTR_EVENT>::iterator bufupd_it;
            for (bufupd_it = pendingBufUpdates_.begin(); bufupd_it != pendingBufUpdates_.end(); ++bufupd_it) {
                deps.push_back(bufupd_it->second);
            }
        } else {
            // Walk over input buffers, grab a completion event for each (if it exists)
            // Let's optimistically assume that each input buffer has been already used in this stream
            deps.reserve(deps.size() + buffers.size());
            std::vector<hStreams_PhysBuffer *>::iterator it;
            for (it = buffers.begin(); it != buffers.end(); ++it) {
                std::map<hStreams_PhysBuffer *, HSTR_EVENT>::iterator bufupd_it = pendingBufUpdates_.find(*it);
                if (bufupd_it != pendingBufUpdates_.end()) {
                    deps.push_back(bufupd_it->second);
                }
            }
        }
    }
}

void hStreams_PhysStream::setOutputDeps(
    DEP_TYPE dep_type,
    std::vector<hStreams_PhysBuffer *> &buffers,
    HSTR_EVENT completion)
{
    // Current implementation conservatively treats RAW, WAW, WAR the same,
    // and even if the dest is the host, treats the read as creating a dep
    // in the hStream.  To check whether all xfers out of a stream are
    // done, one can do a StreamSync.

    if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_CONSERVATIVE) {
        // dep_type is a don't care
        lastAction_ = completion;
    } else if (hStreams_GetOptions_dep_policy() == HSTR_DEP_POLICY_BUFFERS) {
        //  FIXME perf: this can be commented out unless and until computes are
        //  OOO [prove this first]
        if (dep_type != IS_XFER) {
            lastAction_ = completion;
        }

        // NOTE if dep_type == IS_BARRIER, caller is responsible for providing all the possible
        //      physical buffers that can be used in this stream; a physical stream doesn't go
        //      and grab the list of all buffers.
        std::vector<hStreams_PhysBuffer *>::iterator it;
        for (it = buffers.begin(); it != buffers.end(); ++it) {
            pendingBufUpdates_[*it] = completion;
        }
    }
}

HSTR_RESULT hStreams_PhysStream::enqueueFunction(
    std::string func_name,
    std::vector<uint64_t> &scalar_args,
    std::vector<hStreams_PhysBuffer *> &buffer_args,
    std::vector<uint64_t> &buffer_offsets,
    void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
)
{
    if (buffer_args.size() != buffer_offsets.size()) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Internal error. buffer_args.size() != buffer_offsets.size() ["
                << buffer_args.size() << " != " << buffer_offsets.size() << "]";

        return HSTR_RESULT_INTERNAL_ERROR;
    }

    HSTR_EVENT completion;
    {
        // Synchronize the enqueues to the physical stream
        hStreams_Scope_Locker_Unlocker _autolock(lock_);

        hStreams_PhysDomain &phys_dom = log_dom_->getPhysDomain();
        uint64_t sink_addr = phys_dom.fetchSinkFunctionAddress(func_name);
        if (!sink_addr) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "A sink-compiled version of called function "
                    << func_name << " not found on logical domain " << log_dom_->id()
                    << ", physical domain " << phys_dom.id();

            return HSTR_RESULT_BAD_NAME;
        }

        // Here we marshall the arguments to the form used by the thunks
        // This involves translating the addresses to sink-side addresses
        std::vector<uint64_t> marshalled_args;
        // Two for scalar/heap args number
        // One for sink-side function address
        marshalled_args.reserve(2 + scalar_args.size() + buffer_args.size() + 1);
        marshalled_args.push_back(scalar_args.size());
        marshalled_args.push_back(buffer_args.size());

        // scalar args go untouched
        marshalled_args.insert(marshalled_args.end(), scalar_args.begin(), scalar_args.end());

        for (uint32_t idx = 0; idx < buffer_args.size(); ++idx) {
            marshalled_args.push_back(buffer_args[idx]->translateToSinkAddress(buffer_offsets[idx]));
        }

        marshalled_args.push_back(sink_addr);

        std::vector<HSTR_EVENT> input_deps;
        getInputDeps(IS_COMPUTE, buffer_args, input_deps);

        // NULL event handle semantics are different in streams and in COI. Streams
        // have FIFO ordering; NULL completion event in EnqueueCompute/EnqueueData
        // means that the user doesn't care about the completion event, but that
        // doesn't mean the call should be synchronous (as is the semantic in COI).
        //
        // This logic is implemented here.
        HSTR_RESULT hret = impl_enqueueFunction(marshalled_args, input_deps,
                                                ret_val, ret_val_size, &completion);
        if (HSTR_RESULT_SUCCESS != hret) {
            return hret;
        }

        setOutputDeps(IS_COMPUTE, buffer_args, completion);

    } // end of critical section protecting enqueues to the stream

    // Go over each buffer and notify it that there's an action involving it
    for (std::vector<hStreams_PhysBuffer *>::iterator it = buffer_args.begin(); it != buffer_args.end(); ++it) {
        (*it)->addPendingAction(completion);
    }

    // If caller wants to wait for completion, copy back the handle.
    if (ret_event != NULL) {
        *ret_event = completion;
    }

    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT hStreams_PhysStream::enqueueTransfer(
    hStreams_PhysBuffer &dst_buf,
    hStreams_PhysBuffer &src_buf,
    uint64_t dst_offset,
    uint64_t src_offset,
    uint64_t length,
    HSTR_EVENT *ret_event
)
{

    std::vector<hStreams_PhysBuffer *> in_dep_bufs;
    in_dep_bufs.push_back(&dst_buf);
    in_dep_bufs.push_back(&src_buf);
    std::vector<HSTR_EVENT> in_deps;
    HSTR_EVENT completion;

    {
        // Synchronize the enqueues to the physical stream
        hStreams_Scope_Locker_Unlocker _autolock(lock_);
        getInputDeps(IS_XFER, in_dep_bufs, in_deps);

        if (dst_offset == src_offset &&
                dst_buf == src_buf &&
                dst_buf.getLogBuffer().isPropertyFlagSet(HSTR_BUF_PROP_ALIASED)) {
            // Do not perform transfer, only resolve dependences
            HSTR_COIRESULT coires = hStreams_COIWrapper::COIBufferWrite(
                                        hstr_proc.dummy_buf,                    // in_DestBuffer
                                        0,                                      // in_Offset
                                        &hstr_proc.dummy_data,                  // in_pSourceData. Cannot be NULL
                                        1,                                      // in_Length
                                        HSTR_COI_COPY_USE_CPU,                       // in_Type
                                        (int32_t) in_deps.size(),               // total number of dependencies
                                        (in_deps.size()) ? &in_deps[0] : NULL,  // in_pDependencies
                                        &completion);                           // out_pCompletion

            if (coires != HSTR_COI_SUCCESS) {
                HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                        << "Couldn't properly enforce dependencies of an optimized-away transfer "
                        << "within an aliased buffer. :"
                        << hStreams_COIWrapper::COIResultGetName(coires)
                        << ". Source buffer: " << src_buf.getLogBuffer().getStart()
                        << " Destination buffer: " << dst_buf.getLogBuffer().getStart();


                return HSTR_RESULT_REMOTE_ERROR;
            }
        } else {
            // Perform transfer
            HSTR_COIRESULT coires = hStreams_COIWrapper::COIBufferCopy(dst_buf.getCOIhandle(), src_buf.getCOIhandle(),
                                    dst_offset + dst_buf.getPadding(),
                                    src_offset + src_buf.getPadding(),
                                    length,
                                    HSTR_COI_COPY_UNSPECIFIED,
                                    (int32_t) in_deps.size(),
                                    (in_deps.size()) ? &in_deps[0] : NULL,
                                    &completion);

            if (coires != HSTR_COI_SUCCESS) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                        << "A problem encountered while copying data between buffers: "
                        << hStreams_COIWrapper::COIResultGetName(coires);
            }


            if (coires == HSTR_COI_MEMORY_OVERLAP) {
                return HSTR_RESULT_OVERLAPPING_RESOURCES;
            } else if (coires != HSTR_COI_SUCCESS) {
                // FIXME handle the cases more appropriately
                return HSTR_RESULT_REMOTE_ERROR;
            }

            // Only update the output dependencies wrt the destination buffer
            std::vector<hStreams_PhysBuffer *> out_dep_bufs;
            out_dep_bufs.push_back(&dst_buf);
            setOutputDeps(IS_XFER, out_dep_bufs, completion);
        }
    } // End of critical section protecting enqueues to the stream


    // Notify the buffers of actions involving them
    dst_buf.addPendingAction(completion);
    src_buf.addPendingAction(completion);

    // If caller wants to wait for completion, copy back the handle.
    if (ret_event != NULL) {
        *ret_event = completion;
    }

    return HSTR_RESULT_SUCCESS;
}
