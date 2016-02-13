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

#include "hStreams_LogStreamCollection.h"

#include <algorithm>


hStreams_LogStreamCollection::hStreams_LogStreamCollection()
{

}

hStreams_LogStreamCollection::~hStreams_LogStreamCollection()
{

}

void hStreams_LogStreamCollection::addToCollection(hStreams_LogStream *log_dom)
{
    container_.push_back(log_dom);
}

void hStreams_LogStreamCollection::delFromCollection(hStreams_LogStream &log_dom)
{
    Container::iterator it = getIteratorByID(log_dom.id());
    if (it == container_.end()) {
        // in DEBUG, log the issue/fail hard?
        return;
    }
    container_.erase(it);
}

hStreams_LogStream *hStreams_LogStreamCollection::lookupByLogStreamID(HSTR_LOG_STR id)
{
    Container::iterator it = getIteratorByID(id);
    if (it == container_.end()) {
        return NULL;
    }
    return *it;
}

hStreams_LogStreamCollection::Container::iterator
hStreams_LogStreamCollection::getIteratorByID(HSTR_LOG_STR id)
{
    hStreams_LogStreamCollection::Container::iterator ret;
    for (ret = container_.begin(); ret != container_.end(); ++ret) {
        if ((*ret)->id() == id) {
            return ret;
        }
    }
    return container_.end();
}

class BelongsToLogDomainFunctor
{
    hStreams_LogDomain *log_dom_;
public:
    BelongsToLogDomainFunctor(hStreams_LogDomain &log_dom) : log_dom_(&log_dom) {}
    bool operator()(hStreams_LogStream *log_stream)
    {
        return log_stream->getLogDomain().id() == log_dom_->id();
    }
};

void hStreams_LogStreamCollection::delFromCollectionByLogDomain(hStreams_LogDomain &log_dom)
{
    // erase-remove, well suited for vectors
    container_.erase(
        std::remove_if(
            container_.begin(),
            container_.end(),
            BelongsToLogDomainFunctor(log_dom)),
        container_.end());
}

void hStreams_LogStreamCollection::getEventsFromAllStreams(std::vector<HSTR_EVENT> &events)
{
    events.clear();
    events.reserve(container_.size());

    std::vector<HSTR_EVENT> tmp_event_vector;
    for (Container::iterator it = container_.begin(); it != container_.end(); ++it) {
        (*it)->getAllEvents(tmp_event_vector);
        events.insert(events.end(), tmp_event_vector.begin(), tmp_event_vector.end());
    }
}

class DeleteLogStreamFunctor
{
public:
    void operator()(hStreams_LogStream *ptr)
    {
        delete ptr;
    }
};

void hStreams_LogStreamCollection::destroyAllStreams()
{
    std::for_each(container_.begin(), container_.end(), DeleteLogStreamFunctor());
    container_.clear();
}
