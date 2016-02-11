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

#include "hStreams_LogBufferCollection.h"
#include "hStreams_LogBuffer.h"
#include <utility>
#include <algorithm>
#include <iostream>

hStreams_LogBufferCollection::hStreams_LogBufferCollection()
{

}

hStreams_LogBufferCollection::~hStreams_LogBufferCollection()
{

}

hStreams_LogBufferCollection::iterator hStreams_LogBufferCollection::begin()
{
    return container_.begin();
}

hStreams_LogBufferCollection::iterator hStreams_LogBufferCollection::end()
{
    return container_.end();
}

void hStreams_LogBufferCollection::addToCollection(hStreams_LogBuffer *new_buf)
{
    container_.insert(std::make_pair(new_buf->getStart(), new_buf));
}

void hStreams_LogBufferCollection::delFromCollection(hStreams_LogBuffer &buf)
{
    container_.erase(buf.getStart());
}

// Check whether addr falls within left-closed, right-open range [start, start+len)
static bool addr_in_ropen_range(void *start, uint64_t len, void *addr)
{
    return (addr >= start) && (addr < (void *)((uint64_t)start + len));
}

hStreams_LogBuffer *hStreams_LogBufferCollection::lookupLogBuffer(void *addr, uint64_t len, HSTR_OVERLAP_TYPE *overlap)
{
    if (0 == len || container_.empty()) {
        *overlap = HSTR_NO_OVERLAP;
        return NULL;
    }
    // A brief explanation of this logic as it is dense.  We only need to consider lower_bound
    // or its immediate predecessor because of the following:
    //
    // map::lower_bound returns the iterator that is immediately less than
    // or, exactly matches the specified address.  If no node exists that is less than
    // or equal to the address, map::lower_bound return map::end.
    //
    // A. if map::lower_bound() provides an iterator, such that it->first == in_Address, we have
    // obviously found the map entry.
    //
    // B. Sometimes, however an in_Address value is specified that does not exactly match the
    // key value (it->first) but resides within the memory range of a node in the map.  When this
    // occurs, map::lower_bound will return the node that comes immediately after this node.
    //
    // C. if there are no nodes in the tree that are less than or equal to address,
    // map::lower_bound, returns map::end.
    //
    // D. if the given address precedes all of the nodes in the map, map::lower_bound returns
    // begin().
    //
    // An example is good here:
    //
    // Suppose there are only two nodes in the map:
    //
    // Entry    |               |
    // Number:  | Address (key) | length
    // ---------+---------------+----------
    // 1 begin()|    0x1000     | 0x1000
    // 2        |    0x3000     | 0x1000
    // end()    |     NA        |   NA
    // ---------+---------------+----------
    //
    // Let entry number end() correspond to map::end.
    //
    // And, suppose we query the following address values with map::lower_bound.
    // map::lower_bound returns the following entry numbers in the map due to the specified
    // reason A, B, C or D:
    //
    // Address: | Entry Number: | Reason:
    // ---------+---------------+----------
    // 0x1000  |      1        |   A
    // 0x3000  |      2        |   A
    // 0x2000  |      2        |   B
    // 0x3010  |     end()     |   B
    // 0x4010  |     end()     |   C
    // 0x10    |      1        |   D

    void *right_end = (void *)((uint64_t)addr + len - 1);

    Container::iterator itlow = container_.lower_bound(addr);
    if (container_.end() == itlow) {
        // Check whether the last element in the container overlaps the queried range
        Container::iterator prev = itlow;
        --prev;
        if (addr_in_ropen_range(prev->second->getStart(), prev->second->getLen(), addr)) {
            if (addr_in_ropen_range(prev->second->getStart(), prev->second->getLen(), right_end)) {
                *overlap = HSTR_EXACT_OVERLAP;
                return prev->second;
            } else {
                *overlap = HSTR_PARTIAL_OVERLAP;
                return NULL;
            }
        } else {
            *overlap = HSTR_NO_OVERLAP;
            return NULL;
        }
    }


    // we've found a buffer whose start address is _not lower_ than addr
    // First, special case when the start addresses are equal:
    if (addr == itlow->second->getStart()) {
        // Need to check the right end of the queried range, whether it fits in the one we found
        // Note that the right end does not count into the buffer, so we use -1
        if (addr_in_ropen_range(itlow->second->getStart(), itlow->second->getLen(), right_end)) {
            *overlap = HSTR_EXACT_OVERLAP;
            return itlow->second;
        } else {
            *overlap = HSTR_PARTIAL_OVERLAP;
            return NULL;
        }
    }

    // By now, we know that itlow points to a buffer whose start address is higher
    // than the queried start address. Need to check whether the upper end of the queried range
    // falls inside that "next" buffer.
    if (addr_in_ropen_range(itlow->second->getStart(), itlow->second->getLen(), right_end)) {
        *overlap = HSTR_PARTIAL_OVERLAP;
        return NULL;
    }

    // Unless we're at the beginning of the container, we also have to check
    // whether the previous range in the container overlaps this buffer
    // (similar to what has been done for end() special case above and whethre
    // the previous one fully contains the requested range or not
    if (container_.begin() != itlow) {
        --itlow;
        if (addr_in_ropen_range(itlow->second->getStart(), itlow->second->getLen(), addr)) {
            if (addr_in_ropen_range(itlow->second->getStart(), itlow->second->getLen(), right_end)) {
                *overlap = HSTR_EXACT_OVERLAP;
                return itlow->second;
            } else {
                *overlap = HSTR_PARTIAL_OVERLAP;
                return NULL;
            }
        }
    }

    // By here, the queried range does not overlap any range already present in the container
    *overlap = HSTR_NO_OVERLAP;
    return NULL;
}

hStreams_LogBuffer *hStreams_LogBufferCollection::lookupLogBuffer(void *addr)
{
    HSTR_OVERLAP_TYPE dont_care;
    return lookupLogBuffer(addr, 1, &dont_care); // The other overload returns NULL for partial overlap
}

hStreams_LogBuffer *hStreams_LogBufferCollection::lookupLogBuffer(uint64_t addr)
{
    return lookupLogBuffer((void *)addr);
}

namespace
{
class DeleteLogBufferFunctor
{
public:
    void operator()(std::pair<void *const, hStreams_LogBuffer *> &element)
    {
        delete element.second;
    }
};
}

void hStreams_LogBufferCollection::destroyAllBuffers()
{
    std::for_each(container_.begin(), container_.end(), DeleteLogBufferFunctor());
    container_.clear();
}

void hStreams_LogBufferCollection::getAllPhysBuffersForLogDomain(hStreams_LogDomain &log_dom, std::vector<hStreams_PhysBuffer *> &phys_buffers)
{
    phys_buffers.clear();
    for (Container::iterator it = container_.begin(); it != container_.end(); ++it) {
        hStreams_PhysBuffer *phys_buf = it->second->getPhysBufferForLogDomain(log_dom);
        if (NULL != phys_buf) {
            phys_buffers.push_back(phys_buf);
        }
    }
}

void hStreams_LogBufferCollection::processDelLogDomain(hStreams_LogDomain &log_dom)
{
    for (Container::iterator it = container_.begin(); it != container_.end(); ++it) {
        it->second->detachLogDomain(log_dom);
    }
}
