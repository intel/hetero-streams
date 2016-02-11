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
#include "hStreams_PhysDomainCollection.h"
#include <algorithm>

hStreams_PhysDomainCollection::hStreams_PhysDomainCollection()
{

}

hStreams_PhysDomainCollection::~hStreams_PhysDomainCollection()
{

}

void hStreams_PhysDomainCollection::addToCollection(hStreams_PhysDomain *phys_dom)
{
    container_.push_back(phys_dom);
}

void hStreams_PhysDomainCollection::delFromCollection(hStreams_PhysDomain &phys_dom)
{
    Container::iterator it = getIteratorByID(phys_dom.id());
    if (it == container_.end()) {
        // in DEBUG, log the issue/fail hard?
        return;
    }
    container_.erase(it);
}

hStreams_PhysDomain *hStreams_PhysDomainCollection::lookupByPhysDomainID(HSTR_PHYS_DOM id)
{
    Container::iterator it = getIteratorByID(id);
    if (it == container_.end()) {
        return NULL;
    }
    return *it;
}

hStreams_PhysDomainCollection::Container::iterator
hStreams_PhysDomainCollection::getIteratorByID(HSTR_PHYS_DOM id)
{
    for (Container::iterator ret = container_.begin(); ret != container_.end(); ++ret) {
        if ((*ret)->id() == id) {
            return ret;
        }
    }
    return container_.end();
}

/// @todo The homogeneity property of the collection could be maintained incrementally during
///     physical domain addition/removal rather than calculated every time from scratch.
bool hStreams_PhysDomainCollection::isHomogenous() const
{
    if (container_.empty()) {
        return true;
    }
    hStreams_PhysDomain *first_dom = container_.front();
    for (Container::const_iterator it = container_.begin(); it != container_.end(); ++it) {
        hStreams_PhysDomain *compared_dom = *it;
        hStreams_CPUMask compared_max_cpu_mask = compared_dom->getMaxCPUMask();
        if (compared_dom->id() == HSTR_SRC_PHYS_DOMAIN) {
            // Skip the host domain for now, as it's not really usable
            continue;
        }
        // Set homogeneous to false if domains have not the same resources
        if (
            !(compared_dom->isa               == first_dom->isa &&
              compared_dom->core_max_freq_MHz == first_dom->core_max_freq_MHz &&
              compared_dom->available_memory  == first_dom->available_memory &&
              HSTR_CPU_MASK_EQUAL(compared_max_cpu_mask.mask, first_dom->getMaxCPUMask().mask))
        ) {
            return false;
        }
    }
    return true;
}

uint32_t hStreams_PhysDomainCollection::size() const
{
    return (uint32_t)container_.size();
}

class DeletePhysDomainFunctor
{
public:
    void operator()(hStreams_PhysDomain *ptr)
    {
        delete ptr;
    }
};

void hStreams_PhysDomainCollection::destroyAllDomains()
{
    std::for_each(container_.begin(), container_.end(), DeletePhysDomainFunctor());
    container_.clear();
}
