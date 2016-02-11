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

#ifndef HSTREAMS_LOGSTREAMCOLLECTION_H
#define HSTREAMS_LOGSTREAMCOLLECTION_H

#include <vector>

#include "hStreams_LogStream.h"

class hStreams_LogDomain;

/// @brief A class which acts as a store for logical stream objects.
class hStreams_LogStreamCollection
{
    typedef std::vector<hStreams_LogStream *> Container;
    /// @brief Actual underlying container
    ///
    /// @todo Could hide it using PIMPL but is it worth the added effort?
    Container container_;
public:
    hStreams_LogStreamCollection();
    ~hStreams_LogStreamCollection();

    /// @brief Add a new logical stream to the store
    void addToCollection(hStreams_LogStream *);
    /// @brief Remove a logical stream from the store
    void delFromCollection(hStreams_LogStream &);
    /// @brief Drop all the streams in the store, calling \c delete on each of them
    /// @note This deletes the objects, i.e. calls \c delete
    /// @todo This method is a helper workaround, used by \c hStreams_Fini().
    ///     It doesn't semantically belong in here, though -- collections do not _own_
    ///     objects they store so they should not be destroying them. As an alternative,
    ///     one should expose iterators to the collection and optionally a \c delAll()
    ///     method which would _not_ delete the objects but simply remove them from the store.
    void destroyAllStreams();
    /// @brief Lookup a logical stream by its id.
    /// @return A pointer to the logical stream, NULL if not found.
    hStreams_LogStream *lookupByLogStreamID(HSTR_LOG_STR id);
    /// @brief Remove all logical streams from a logical domain from the store
    void delFromCollectionByLogDomain(hStreams_LogDomain &);
    /// @brief Obtain all events ever used in any of the streams
    /// @sa hStreams_ThreadSynchronize()
    void getEventsFromAllStreams(std::vector<HSTR_EVENT> &events);
private:
    /// @brief Helper lookup function, used internally
    /// @return iterator to the position in the container if \c id is valid,
    ///    \c container_.end() otherwise
    Container::iterator getIteratorByID(HSTR_LOG_STR id);

    // copy/assignment construction disallowed
    hStreams_LogStreamCollection &operator=(hStreams_LogStreamCollection const &other);
    hStreams_LogStreamCollection(hStreams_LogStreamCollection const &other);
};


#endif /* HSTREAMS_LOGSTREAMCOLLECTION_H */
