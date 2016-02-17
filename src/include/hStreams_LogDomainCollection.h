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

#ifndef HSTREAMS_LOGDOMAINCOLLECTION_H
#define HSTREAMS_LOGDOMAINCOLLECTION_H

#include <vector>

#include "hStreams_LogDomain.h"

/// @brief A class which acts as a store for logical domain objects.
class hStreams_LogDomainCollection
{
    typedef std::vector<hStreams_LogDomain *> Container;
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
    ///
    /// @todo This should be implemented as a class inheriting from
    ///     std::iterator<std::bidirectional_iterator_tag, hStreams_LogDomain*> or
    ///     similar to make it interoperable with stl iterators (e.g. to be able
    ///     to use the same algorithms for logical domains contained in a set/map
    ///     or in this store without making the algorithms fully templated)
    ///
    /// @todo Instead of relying on the index from the vector, this class could be well
    ///     written to contain Container::iterator as its private member.
    class iterator
    {
        friend class hStreams_LogDomainCollection;
    public:
        iterator() : container_(NULL), idx_(0) {}
        iterator(Container &container, uint32_t idx) : container_(&container), idx_(idx) {}
        /// @brief Obtain the element from under the iterator
        hStreams_LogDomain *&operator*()
        {
            return container_->at(idx_);
        }
        /// @brief Move the iterator forward
        iterator &operator++()
        {
            ++idx_;
            return *this;
        }
        /// @brief Move the iterator back
        iterator &operator--()
        {
            --idx_;
            return *this;
        }
        /// @brief Compare two iterators
        bool operator==(iterator const &rhs) const
        {
            return (idx_ == rhs.idx_) && (container_ == rhs.container_);
        }
        /// @brief Contrast two iterators
        bool operator!=(iterator const &rhs) const
        {
            return !(*this == rhs);
        }
    private:
        /// @brief Index into the logical domain store's underlying container (vector)
        ///
        /// @note See a todo note in the class description regarding this.
        uint32_t idx_;
        /// @brief A "link" to the logical domain store's underlying container
        Container *container_;
    };
    hStreams_LogDomainCollection();
    ~hStreams_LogDomainCollection();

    iterator begin();
    iterator end();

    /// @brief Add a new logical domain to the store
    void addToCollection(hStreams_LogDomain *);
    /// @brief Remove a logical domain from the store
    /// @note Removing a logical domain from the store is considered to invalidate
    ///     any previously obtained iterators to the collection.
    /// @todo Should a similar interface be exposed, one accepting an iterator
    ///     instead of a logical domain?
    void delFromCollection(hStreams_LogDomain &);
    /// @brief Drop all the domains in the store, calling \c delete on each of them
    /// @note This deletes the objects, i.e. calls \c delete
    /// @note This member function is provided to make the interfaces of the collections
    ///     uniform.
    void destroyAllDomains();
    /// @brief Lookup a logical domain by its id.
    /// @return A pointer to the logical domain, NULL if not found.
    hStreams_LogDomain *lookupByLogDomainID(HSTR_LOG_DOM id);
private:
    /// @brief Helper lookup function, used internally
    /// @return iterator to the position in the container if \c id is valid,
    ///    \c container_.end() otherwise
    Container::iterator getIteratorByID(HSTR_LOG_DOM id);

    // copy/assignment construction disallowed
    hStreams_LogDomainCollection &operator=(hStreams_LogDomainCollection const &other);
    hStreams_LogDomainCollection(hStreams_LogDomainCollection const &other);
};

#endif /* HSTREAMS_LOGDOMAINCOLLECTION_H */
