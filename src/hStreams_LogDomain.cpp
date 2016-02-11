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
#include "hStreams_LogStream.h"

#include <algorithm>
#include <string.h>

hStreams_LogDomain::hStreams_LogDomain(HSTR_LOG_DOM id, hStreams_CPUMask const &my_cpu_mask, hStreams_PhysDomain &phys_dom)
    : id_(id), phys_dom_(&phys_dom), cpu_mask_(my_cpu_mask)
{
}

hStreams_LogDomain::~hStreams_LogDomain()
{

}

hStreams_LogStream *hStreams_LogDomain::lookupLogStreamByCPUMask(hStreams_CPUMask const &mask, HSTR_OVERLAP_TYPE *out_overlap) const
{
    HSTR_CPU_MASK tmp_mask;
    HSTR_OVERLAP_TYPE tmp_overlap = HSTR_NO_OVERLAP;
    for (LogStreamsContainer::const_iterator it = log_streams_.begin(); it != log_streams_.end(); ++it) {
        // check for full overlap
        if (mask == (*it)->getCPUMask()) {
            if (out_overlap) {
                *out_overlap = HSTR_EXACT_OVERLAP;
            }
            return *it;
        }
        HSTR_CPU_MASK_AND(tmp_mask, mask.mask, (*it)->getCPUMask().mask);
        if (HSTR_CPU_MASK_COUNT(tmp_mask)) {
            tmp_overlap = HSTR_PARTIAL_OVERLAP;
            // can't stop here as even though there's a partially overlapping element
            // there may be an exactly overlapping element further down the container
        }
    }

    // By here, no exactly overlapping element has been found
    if (out_overlap) {
        *out_overlap = tmp_overlap;
    }
    return NULL;
}

void hStreams_LogDomain::getLogStreamIDs(HSTR_LOG_STR *ptr, uint32_t num, uint32_t *num_written, uint32_t *num_present) const
{
    uint32_t index = 0;
    for (LogStreamsContainer::const_iterator it = log_streams_.begin(); it != log_streams_.end(); ++it, ++index) {
        //Don't fill after array ends
        if (index == num) {
            break;
        }

        ptr[index] = (*it)->id();
    }

    *num_written = index;
    *num_present = (uint32_t) log_streams_.size();
}

uint32_t hStreams_LogDomain::getNumLogStreams() const
{
    return (uint32_t) log_streams_.size();
}

hStreams_CPUMask hStreams_LogDomain::getOccupiedStreamCPUMask() const
{
    hStreams_CPUMask ret;
    LogStreamsContainer::const_iterator it;
    for (it = log_streams_.begin(); it != log_streams_.end(); ++it) {
        HSTR_CPU_MASK_OR(ret.mask, ret.mask, (*it)->getCPUMask().mask);
    }
    return ret;
}

class DeleteLogStreamFunctor
{
public:
    void operator()(hStreams_LogStream *ptr)
    {
        delete ptr;
    }
};

void hStreams_LogDomain::destroyAllStreams()
{
    std::for_each(log_streams_.begin(), log_streams_.end(), DeleteLogStreamFunctor());
    log_streams_.clear();
}

void hStreams_LogDomain::delLogStreamMapping(hStreams_LogStream &log_str)
{
    LogStreamsContainer::iterator it;
    for (it = log_streams_.begin(); it != log_streams_.end(); ++it) {
        if ((*it)->id() == log_str.id()) {
            log_streams_.erase(it);
            return;
        }
    }
    // TODO log the failure? Report it in DEBUG version of the library?
}

void hStreams_LogDomain::addLogStreamMapping(hStreams_LogStream &log_str)
{
    log_streams_.push_back(&log_str);
}

hStreams_CPUMask hStreams_LogDomain::getCPUMask() const
{
    return cpu_mask_;
}
