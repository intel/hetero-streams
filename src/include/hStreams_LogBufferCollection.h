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

#ifndef HSTREAMS_LOGBUFFERCOLLECTION_H
#define HSTREAMS_LOGBUFFERCOLLECTION_H

#include <map>
#include <vector>

#include "hStreams_types.h"

class hStreams_LogBuffer;
class hStreams_LogDomain;
class hStreams_PhysBuffer;

/// @brief A class which acts as a store for logical buffer objects.
class hStreams_LogBufferCollection
{
    typedef std::map<void *, hStreams_LogBuffer *> Container;
    /// @brief Actual underlying container
    ///
    /// @todo Could hide it using PIMPL but is it worth the added effort?
    Container container_;
public:
    /// @brief An iterator over entries in the store
    ///
    /// The rationale behind exposing a custom iterator is encapsulating the
    /// underlying method of storing the individual logical domains so that in the
    /// future it can be adjusted as needed (e.g. if different lookup methods
    /// are deemed important)
    class iterator
    {
        friend class hStreams_LogBufferCollection;
    public:
        iterator() {}
        iterator(Container::iterator it) : it_(it) {}
        /// @brief Obtain the element from under the iterator
        hStreams_LogBuffer *&operator*()
        {
            return it_->second;
        }
        /// @brief Move the iterator forward
        iterator &operator++()
        {
            ++it_;
            return *this;
        };
        /// @brief Move the iterator back
        iterator &operator--()
        {
            --it_;
            return *this;
        };
        /// @brief Compare two iterators
        bool operator==(iterator const &rhs) const
        {
            return (it_ == rhs.it_);
        }
        /// @brief Contrast two iterators
        bool operator!=(iterator const &rhs) const
        {
            return !(*this == rhs);
        }
    private:
        Container::iterator it_;
    };

    hStreams_LogBufferCollection();
    ~hStreams_LogBufferCollection();

    iterator begin();
    iterator end();

    /// @brief Add a new logical buffer to the store
    void addToCollection(hStreams_LogBuffer *);
    /// @brief Remove a logical buffer from the store
    void delFromCollection(hStreams_LogBuffer &);

    /// @brief Drop all the buffers in the store, calling \c delete on each of them
    /// @note This deletes the objects, i.e. calls \c delete
    /// @todo This method is a helper workaround, used by \c hStreams_Fini().
    ///     It doesn't semantically belong in here, though -- collections do not _own_
    ///     objects they store so they should not be destroying them. As an alternative,
    ///     one should expose iterators to the collection and optionally a \c delAll()
    ///     method which would _not_ delete the objects but simply remove them from the store.
    void destroyAllBuffers();
    /// @brief Lookup a logical buffer by its host proxy address
    /// @note The address can fall anywhere inside [buf_start ; buf_start + buf_len)
    /// @return A pointer to the logical buffer, NULL if not found
    hStreams_LogBuffer *lookupLogBuffer(void *addr);
    /// @brief Lookup a logical buffer by its host proxy address (uint64_t version)
    /// @note The address can fall anywhere inside [buf_start ; buf_start + buf_len)
    /// @note This is only for convenience, as sometimes the buffer addresses are passed
    ///     to the API as uint64_t rather than void*
    /// @return A pointer to the logical buffer, NULL if not found
    hStreams_LogBuffer *lookupLogBuffer(uint64_t addr);

    /// @brief Lookup a logical buffer by its host proxy address range
    /// @note [addr,addr+len) can fall anywhere iniside a logical buffer for a
    ///     successful lookup
    /// @param[in] addr the start address of the range to look up
    /// @param[in] len the length of the range to look up
    /// @param[out] overlap Information on whether this range overlaps some logical buffer or not
    ///     It can be set to:
    ///      - NO OVERLAP if not found, additionally NULL is returned as the buffer
    ///      - EXACT_OVERLAP if [start,start+len) is contained within some buffer (this
    ///        includes the special case of exact overlap)
    ///      - PARTIAL_OVERLAP if [start,start+len) partially overlaps some other buffer;
    ///        Additionally, NULL is returned as the logical buffer pointer
    /// @return A pointer to the logical buffer, NULL if not found
    hStreams_LogBuffer *lookupLogBuffer(void *addr, uint64_t len, HSTR_OVERLAP_TYPE *overlap);
    /// @brief Go over all buffers, calling detachLogDomain method on each
    void processDelLogDomain(hStreams_LogDomain &);
    /// @brief Obtain a vector of all physical buffers in a given logical domain
    /// @sa hStreams_PhysStream::setOutputDeps()
    /// @sa hStreams_EventStreamWait()
    void getAllPhysBuffersForLogDomain(hStreams_LogDomain &log_dom, std::vector<hStreams_PhysBuffer *> &phys_buffers);
private:
    // copy/assignment construction disallowed
    hStreams_LogBufferCollection &operator=(hStreams_LogBufferCollection const &other);
    hStreams_LogBufferCollection(hStreams_LogBufferCollection const &other);
};


#endif /* HSTREAMS_LOGBUFFERCOLLECTION_H */
