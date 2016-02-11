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

#ifndef HSTREAMS_PHYSDOMAINCOLLECTION_H
#define HSTREAMS_PHYSDOMAINCOLLECTION_H

#include <vector>

#include "hStreams_PhysDomain.h"

/// @brief A class which acts as a store for physical domains objects.
class hStreams_PhysDomainCollection
{
    typedef std::vector<hStreams_PhysDomain *> Container;
    /// @brief Actual underlying container
    ///
    /// @todo Could hide it using PIMPL but is it worth the added effort?
    Container container_;
public:
    hStreams_PhysDomainCollection();
    ~hStreams_PhysDomainCollection();

    /// @brief Add a new physical domain to the store
    void addToCollection(hStreams_PhysDomain *);
    /// @brief Remove a physical domain from the store
    void delFromCollection(hStreams_PhysDomain &);
    /// @brief Drop all the domains in the store, calling \c delete on each of them
    /// @note This deletes the objects, i.e. calls \c delete
    /// @todo This method is a helper workaround, used by \c hStreams_Fini().
    ///     It doesn't semantically belong in here, though -- collections do not _own_
    ///     objects they store so they should not be destroying them. As an alternative,
    ///     one should expose iterators to the collection and optionally a \c delAll()
    ///     method which would _not_ delete the objects but simply remove them from the store.
    void destroyAllDomains();
    /// @brief Lookup a physical domain by its id.
    /// @return A pointer to the physical domain, NULL if not found.
    hStreams_PhysDomain *lookupByPhysDomainID(HSTR_PHYS_DOM id);
    /// @brief Check whether all the physical domains in the store have the same traits.
    ///
    /// The traits which are considered for homo/heter-geneity are:
    /// - ISA
    /// - maximum frequency of a core
    /// - available memory amount
    /// - max cpu mask
    bool isHomogenous() const;
    /// @brief Get number of elements in the collection
    uint32_t size() const;
private:
    /// @brief Helper lookup function, used internally
    /// @return iterator to the position in the container if \c id is valid,
    ///    \c container_.end() otherwise
    Container::iterator getIteratorByID(HSTR_PHYS_DOM id);

    // copy/assignment construction disallowed
    hStreams_PhysDomainCollection &operator=(hStreams_PhysDomainCollection const &other);
    hStreams_PhysDomainCollection(hStreams_PhysDomainCollection const &other);
};


#endif /* HSTREAMS_PHYSDOMAINCOLLECTION_H */
