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

#ifndef HSTREAMS_LOGBUFFER_H
#define HSTREAMS_LOGBUFFER_H

#include <map>
#include "hStreams_Logger.h"

class hStreams_LogDomain;
class hStreams_PhysBuffer;

/// @brief A class which represents an individual logical buffer
/// @sa hStreams_PhysBuffer
///
/// A logical buffer is an entity enumerable by the user. A logical buffer does not
/// own any memory resources itself. A logical buffer uses physical buffers for that purpose.
///
/// Implementation-wise, a logical buffer _attaches_ to a physical buffer, thus
/// increasing the physical buffer's refcount.
class hStreams_LogBuffer
{
    typedef std::map<const hStreams_LogDomain *, hStreams_PhysBuffer *> PhysBufferContainer;
    /// @brief A collection of physical buffers this logical buffer is attached to.
    PhysBufferContainer phys_buffers_;
    /// @brief The start address of the buffer in the source proxy address space
    ///
    /// This comes from what the user supplied to the \c hStreams_Alloc1D() or
    /// \c hStreams_Alloc1DEx() APIs
    void *const start_;
    /// @brief The length of the buffer in the source proxy address space
    ///
    /// This comes from what the user supplied to the \c hStreams_Alloc1D or
    /// \c hStreams_Alloc1DEx APIs
    const uint64_t len_;
    /// @brief The properties of the buffer
    const HSTR_BUFFER_PROPS properties_;
    /// @brief Offset into the cache line on the source proxy address space
    ///
    /// It is used by physical buffers to calculate address translation for sink buffers.
    uint64_t offset_;
public:
    /// @param[in] start The start address of the buffer in the source proxy address space.
    /// @param[in] len Size of buffer.
    /// @param[in] properties Buffer properties.
    /// @sa hStreams_Alloc1D
    /// @sa hStreams_Alloc1DEx
    hStreams_LogBuffer(void *start, uint64_t len, const HSTR_BUFFER_PROPS &properties);
    ~hStreams_LogBuffer();
    /// @brief Get the start address of the buffer in the source proxy address space
    void *getStart() const;
    /// @brief Get the start address of the buffer in the source proxy address space
    /// @note This is only for convenience, as sometimes the buffer addresses are passed
    ///     to the API as uint64_t rather than void*
    uint64_t getStartu64() const;
    /// @brief Get the length of the logical buffer, in bytes
    uint64_t getLen() const;
    /// @brief Get buffer properties
    HSTR_BUFFER_PROPS getProperties() const;
    /// @brief Return true if property flag is set
    /// @param[in] flag Flag to check out
    /// @sa HSTR_BUFFER_PROP_FLAGS
    bool isPropertyFlagSet(const uint64_t flag) const;
    /// @brief number of attached log domains
    /// @note the instance for the \c HSTR_SRC_LOG_DOMAIN is not counted here.
    ///
    /// This function will return \c 0 for both a logical buffer which doesn't have any
    /// instantiations at all and a logical buffer which is only instantiated for the
    /// source logical domain
    uint64_t getNumAttachedLogDomains();
    /// @brief Get attached logical domains' IDs
    /// @param[in] maxNumLogDomains size of pLogDomains table
    /// @param[out] pLogDomains table of attached log domains' IDs
    /// @param[out] numPresent number of attached log domains
    /// HSTR_SRC_LOG_DOMAIN is not include to results.
    /// @note Function assume that pLogDomains have allocated enough space for at least
    /// MaxNumLogDomains log domains' IDs
    void getAttachedLogDomainIDs(uint64_t MaxNumLogDomains, HSTR_LOG_DOM *pLogDomains, uint64_t *numPresent);
    /// @brief Create instances for a logical domain that already exists
    ///
    /// A hook, usually used after creating a brand new logical buffers, to create instances
    /// in already present logical domains.
    ///
    /// @note The source logical domain must be EXPLICITLY attached to the logical buffer
    HSTR_RESULT attachExistingLogDomain(const hStreams_LogDomain &);
    /// @brief Attach all log domains from given range
    /// @param[in] first Iterator to first element of range
    /// @param[in] last Iterator to next element after last element
    template<class iterator>
    HSTR_RESULT attachLogDomainFromRange(iterator first, iterator last);
    /// @brief Remove instantiation of a logical buffer for some logical domain
    ///
    /// A hook for processing the fact that a logical buffer should no longer have an
    /// instantiation for a given logical domain.
    ///
    /// @note This merely detaches from the underlying physical buffer for this logical domain
    ///     which means that in case of aliased buffers, this may or may not cause a removal
    ///     of the actual COI buffer.
    void detachLogDomain(const hStreams_LogDomain &);
    /// @brief Detach all log domains from given range
    /// Range including the element pointed by first but not the element pointed by last
    /// @param[in] first Iterator to first element of range
    /// @param[in] last Iterator to next element after last element
    template<class iterator>
    void detachLogDomainFromRange(iterator first, iterator last);
    /// @brief Detach all attached log domain
    void detachAllLogDomain();
    /// @brief Obtain a physical buffer instance for a given logical domain.
    /// @return A pointer to the physical buffer or NULL if this logical buffer is
    ///     not instantiated for this logical domain
    hStreams_PhysBuffer *getPhysBufferForLogDomain(const hStreams_LogDomain &);
private:
    /// @brief Try attach existing phys buffer to log domain on the same phys domain
    /// @return true if buffer was found and attached
    /// @return false if buffer instance doesn't exist on this phys domain
    bool tryAttachToAliasingBuffer(const hStreams_LogDomain &log_dom);
    // assingment prohibited
    hStreams_LogBuffer &operator=(hStreams_LogBuffer const &other);
};

template<class iterator>
HSTR_RESULT hStreams_LogBuffer::attachLogDomainFromRange(iterator first, iterator last)
{
    iterator it;
    for (it = first; it != last; ++it) {
        HSTR_RESULT hret = attachExistingLogDomain(*(*it));
        if (hret != HSTR_RESULT_SUCCESS) {
            HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                    << "Could not instantiate buffer " << (void *) start_ << " for logical domain "
                    << (*it)->id() << ": " << hStreams_ResultGetName(hret);

            // Revert changes if any error occurred
            detachLogDomainFromRange(first,  it);
            return hret;
        }
    }
    return HSTR_RESULT_SUCCESS;
}

template<class iterator>
void hStreams_LogBuffer::detachLogDomainFromRange(iterator first, iterator last)
{
    iterator it;
    for (it = first; it != last; ++it) {
        detachLogDomain(*(*it));
    }
}

#endif /* HSTREAMS_LOGBUFFER_H */
