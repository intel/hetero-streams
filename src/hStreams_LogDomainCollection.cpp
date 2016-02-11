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

#include "hStreams_LogDomainCollection.h"
#include <algorithm>

hStreams_LogDomainCollection::hStreams_LogDomainCollection()
{

}

hStreams_LogDomainCollection::~hStreams_LogDomainCollection()
{

}

hStreams_LogDomainCollection::iterator hStreams_LogDomainCollection::begin()
{
    return iterator(container_, 0);
}

hStreams_LogDomainCollection::iterator hStreams_LogDomainCollection::end()
{
    return iterator(container_, (uint32_t) container_.size());
}

void hStreams_LogDomainCollection::addToCollection(hStreams_LogDomain *log_dom)
{
    container_.push_back(log_dom);
}

void hStreams_LogDomainCollection::delFromCollection(hStreams_LogDomain &log_dom)
{
    Container::iterator it = getIteratorByID(log_dom.id());
    if (it == container_.end()) {
        // in DEBUG, log the issue/fail hard?
        return;
    }
    container_.erase(it);
}

hStreams_LogDomain *hStreams_LogDomainCollection::lookupByLogDomainID(HSTR_LOG_DOM id)
{
    Container::iterator it = getIteratorByID(id);
    if (it == container_.end()) {
        return NULL;
    }
    return *it;
}

hStreams_LogDomainCollection::Container::iterator
hStreams_LogDomainCollection::getIteratorByID(HSTR_LOG_DOM id)
{
    hStreams_LogDomainCollection::Container::iterator ret;
    for (ret = container_.begin(); ret != container_.end(); ++ret) {
        if ((*ret)->id() == id) {
            return ret;
        }
    }
    return container_.end();
}

class DeleteLogDomainFunctor
{
public:
    void operator()(hStreams_LogDomain *ptr)
    {
        delete ptr;
    }
};

void hStreams_LogDomainCollection::destroyAllDomains()
{
    std::for_each(container_.begin(), container_.end(), DeleteLogDomainFunctor());
    container_.clear();
}
