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

#include "hStreams_internal.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_types.h"
#include "hStreams_Logger.h"
#include "hStreams_LogBuffer.h"
#include "hStreams_PhysBuffer.h"
#include "hStreams_PhysBufferHost.h"

#include <utility>
#include <stdlib.h>
#include <memory>

hStreams_LogBuffer::hStreams_LogBuffer(void *start, uint64_t len, const HSTR_BUFFER_PROPS &properties)
    : start_(start), len_(len), offset_((uint64_t)(start) % 64), properties_(properties)
{

}

hStreams_LogBuffer::~hStreams_LogBuffer()
{
    detachAllLogDomain();
}

void *hStreams_LogBuffer::getStart() const
{
    return start_;
}
uint64_t hStreams_LogBuffer::getStartu64() const
{
    return (uint64_t)getStart();
}
uint64_t hStreams_LogBuffer::getLen() const
{
    return len_;
}
HSTR_BUFFER_PROPS hStreams_LogBuffer::getProperties() const
{
    return properties_;
}

namespace
{
HSTR_RESULT createSourceCOIBUFFER(const hStreams_LogBuffer &log_buff, void *mem, uint64_t len, HSTR_COIPROCESS coi_proc, HSTR_COIBUFFER *out_coi_buf)
{
    HSTR_COIRESULT coires;
    uint32_t flags = 0;

    if (!log_buff.isPropertyFlagSet(HSTR_BUF_PROP_SRC_PINNED)) {
        // If not pinned from the start, defer pinning until on-access demand
        flags |= HSTR_COI_OPTIMIZE_NO_DMA;
    }

    coires = hStreams_COIWrapper::COIBufferCreateFromMemory(len, HSTR_COI_BUFFER_NORMAL, flags, mem, 1,
             &coi_proc, out_coi_buf);

    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create host-side buffer [" << log_buff.getStart()
                << "]: " << hStreams_COIWrapper::COIResultGetName(coires);

        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT createSinkCOIBUFFER(uint64_t len, HSTR_COIPROCESS coi_proc, uint64_t *out_sink_addr, HSTR_COIBUFFER *out_coi_buf)
{
    HSTR_COIRESULT coires;
    uint32_t flags = HSTR_COI_OPTIMIZE_NO_DMA;

    if (huge_page_usage_threshold != -1 && len > (uint64_t)huge_page_usage_threshold) {
        flags |= HSTR_COI_OPTIMIZE_HUGE_PAGE_SIZE;
    }

    coires = hStreams_COIWrapper::COIBufferCreate(len, HSTR_COI_BUFFER_NORMAL, flags, NULL, 1, &coi_proc,
             out_coi_buf);
    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create sink-side buffer: " << hStreams_COIWrapper::COIResultGetName(coires);

        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    // Materialize on domain
    // Completion not tracked since alloc is blocking - REVIEW
    coires = hStreams_COIWrapper::COIBufferSetState(*out_coi_buf, coi_proc, HSTR_COI_BUFFER_VALID,
             HSTR_COI_BUFFER_NO_MOVE, 0, NULL, NULL);
    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create sink side buffer(sink-side instantiation): "
                << hStreams_COIWrapper::COIResultGetName(coires);

        // cleanup the buffer
        HSTR_COIRESULT destroy_res = hStreams_COIWrapper::COIBufferDestroy(*out_coi_buf);
        if (destroy_res != HSTR_COI_SUCCESS) {
            // we don't return a different error code if destruction fails
            // Is this really needed ?
            HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                    << "Couldn't destroy sink-side buffer: " << hStreams_COIWrapper::COIResultGetName(coires);
        }
        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    // Invalidate host-side instance of this buffer so it's not used as a source for
    //  device -> host copies.  All buffers start out as valid on the host upon creation.
    // Completion not tracked since alloc is blocking - REVIEW
    coires = hStreams_COIWrapper::COIBufferSetState(*out_coi_buf, HSTR_COI_PROCESS_SOURCE, HSTR_COI_BUFFER_INVALID,
             HSTR_COI_BUFFER_NO_MOVE, 0, NULL, NULL);
    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create sink-side buffer (removing host instantion): "
                << hStreams_COIWrapper::COIResultGetName(coires);

        // cleanup the buffer
        HSTR_COIRESULT destroy_res = hStreams_COIWrapper::COIBufferDestroy(*out_coi_buf);
        if (destroy_res != HSTR_COI_SUCCESS) {
            // we don't return a different error code if destruction fails
            HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                    << "Couldn't destroy sink-side buffer: " << hStreams_COIWrapper::COIResultGetName(coires);
        }
        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    coires = hStreams_COIWrapper::COIBufferAddRefcnt(coi_proc, *out_coi_buf, 1);
    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create sink-side buffer (increasing sink-side refcount): "
                << hStreams_COIWrapper::COIResultGetName(coires);

        // cleanup the buffer
        HSTR_COIRESULT destroy_res = hStreams_COIWrapper::COIBufferDestroy(*out_coi_buf);
        if (destroy_res != HSTR_COI_SUCCESS) {
            // we don't return a different error code if destruction fails
            HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                    << "Couldn't destroy sink-side buffer: " << hStreams_COIWrapper::COIResultGetName(coires);
        }
        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    uint64_t sink_addr;
    coires = hStreams_COIWrapper::COIBufferGetSinkAddress(*out_coi_buf, &sink_addr);
    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create sink-side buffer (obtaining address): "
                << hStreams_COIWrapper::COIResultGetName(coires);

        // cleanup the buffer
        HSTR_COIRESULT destroy_res = hStreams_COIWrapper::COIBufferDestroy(*out_coi_buf);
        if (destroy_res != HSTR_COI_SUCCESS) {
            // we don't return a different error code if destruction fails
            HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                    << "Couldn't destroy sink-side buffer: " << hStreams_COIWrapper::COIResultGetName(coires);
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    if (out_sink_addr) {
        *out_sink_addr = sink_addr;
    }
    return HSTR_RESULT_SUCCESS;
}

HSTR_RESULT createHostSideCOIBUFFER(void *mem, uint64_t len,
                                    HSTR_COIPROCESS coi_proc, HSTR_COIBUFFER *out_coi_buf)
{
    HSTR_COIRESULT coires;
    uint32_t flags = 0;

    if (huge_page_usage_threshold != -1 && len > (uint64_t)huge_page_usage_threshold) {
        flags |= HSTR_COI_OPTIMIZE_HUGE_PAGE_SIZE;
    }

    coires = hStreams_COIWrapper::COIBufferCreateFromMemory(len, HSTR_COI_BUFFER_NORMAL, 0, mem, 1,
             &coi_proc, out_coi_buf);

    if (coires != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MEM)
                << "Couldn't create host-side buffer: "  << hStreams_COIWrapper::COIResultGetName(coires);

        if (coires == HSTR_COI_OUT_OF_MEMORY || coires == HSTR_COI_RESOURCE_EXHAUSTED) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }
        return HSTR_RESULT_REMOTE_ERROR;
    }

    return HSTR_RESULT_SUCCESS;
}


} // anoynous namespace

HSTR_RESULT hStreams_LogBuffer::attachExistingLogDomain(const hStreams_LogDomain &log_dom)
{
    PhysBufferContainer::iterator it = phys_buffers_.find(&log_dom);
    if (phys_buffers_.end() != it) {
        return HSTR_RESULT_ALREADY_FOUND;
    }
    // First check whether the buffer's an aliasing one. If yes, search for
    // another physbuffer in the logdomain's physdomain and attach to that.
    if (isPropertyFlagSet(HSTR_BUF_PROP_ALIASED)) {
        if (tryAttachToAliasingBuffer(log_dom)) {
            return HSTR_RESULT_SUCCESS;
        }
    }

    hStreams_PhysDomain &phys_dom = log_dom.getPhysDomain();

    HSTR_COIBUFFER coi_buf;
    hStreams_PhysBuffer *new_buffer;
    if (log_dom.id() == HSTR_SRC_LOG_DOMAIN) {
        CHECK_HSTR_RESULT(createSourceCOIBUFFER(*this, start_, len_, phys_dom.getCOIProcess(), &coi_buf));
        new_buffer = new hStreams_PhysBuffer(*this, coi_buf, start_, 0); // source log domain with offset 0
    } else if (phys_dom.id() == HSTR_SRC_PHYS_DOMAIN) {
        void *mem = NULL;
        uint64_t compensated_len = len_ + offset_;

        mem = hStreams_MemAlignedAllocator::alloc(compensated_len);

        if (mem == NULL) {
            return HSTR_RESULT_OUT_OF_MEMORY;
        }

        CHECK_HSTR_RESULT(createHostSideCOIBUFFER(mem, compensated_len, phys_dom.getCOIProcess(), &coi_buf));

        std::unique_ptr<void, void(*)(void *)> data_ptr(mem, hStreams_MemAlignedAllocator::dealloc);
        new_buffer = new hStreams_PhysBufferHost(*this, coi_buf, std::move(data_ptr), offset_);
    } else {
        uint64_t sink_addr;
        CHECK_HSTR_RESULT(createSinkCOIBUFFER(len_ + offset_, phys_dom.getCOIProcess(), &sink_addr, &coi_buf));
        new_buffer = new hStreams_PhysBuffer(*this, coi_buf, sink_addr, offset_);
    }
    phys_buffers_[&log_dom] = new_buffer;
    return HSTR_RESULT_SUCCESS;
}

bool hStreams_LogBuffer::tryAttachToAliasingBuffer(const hStreams_LogDomain &log_dom)
{
    hStreams_PhysDomain &phys_dom = log_dom.getPhysDomain();
    PhysBufferContainer::iterator it;

    for (it = phys_buffers_.begin(); it != phys_buffers_.end(); ++it) {
        if (it->first->getPhysDomain().id() == phys_dom.id()) {
            hStreams_PhysBuffer *phys_buf = it->second;
            phys_buf->attach();
            phys_buffers_[&log_dom] = phys_buf;
            return true;
        }
    }
    return false;
}

void hStreams_LogBuffer::detachLogDomain(const hStreams_LogDomain &log_dom)
{
    PhysBufferContainer::iterator it = phys_buffers_.find(&log_dom);
    if (phys_buffers_.end() == it) {
        return;
    }
    hStreams_PhysBuffer *phys_buf = it->second;
    phys_buffers_.erase(it);
    phys_buf->detach();
}

void hStreams_LogBuffer::detachAllLogDomain()
{
    for (PhysBufferContainer::iterator it = phys_buffers_.begin(); it != phys_buffers_.end(); ++it) {
        it->second->detach();
    }
    phys_buffers_.clear();
}

hStreams_PhysBuffer *hStreams_LogBuffer::getPhysBufferForLogDomain(const hStreams_LogDomain &log_dom)
{
    PhysBufferContainer::iterator it = phys_buffers_.find(&log_dom);
    if (phys_buffers_.end() == it) {
        return NULL;
    }
    return it->second;
}

bool hStreams_LogBuffer::isPropertyFlagSet(const uint64_t flag) const
{
    return (properties_.flags & flag) == flag;
}

uint64_t hStreams_LogBuffer::getNumAttachedLogDomains()
{
    if (phys_buffers_.empty()) {
        return 0;
    }
    return phys_buffers_.size() - 1;
}

void hStreams_LogBuffer::getAttachedLogDomainIDs(uint64_t numMax, HSTR_LOG_DOM *pLogDomains, uint64_t *numPresent)
{
    PhysBufferContainer::iterator it;
    uint64_t i = 0;
    // Skip HSTR_SRC_LOG_DOMAIN
    for (it = phys_buffers_.begin(); it != phys_buffers_.end(); ++it) {
        if (i >= numMax) {
            break;
        }
        if (it->first->id() != HSTR_SRC_LOG_DOMAIN) {
            pLogDomains[i++] = it->first->id();
        }
    }

    *numPresent = getNumAttachedLogDomains();
}
