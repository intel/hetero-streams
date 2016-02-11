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

#ifndef HSTREAMS_LOGDOMAIN_H
#define HSTREAMS_LOGDOMAIN_H

#include "hStreams_PhysDomain.h"

class hStreams_LogStream;

/// @brief A class which represents an individual logical domain
///
/// A logical domain is an entity enumerable by the user. It is contained inside
/// a physical domain; there may be a many to one mapping of logical domains to
/// physical domains. Moreover, there may be a many to one mapping of logical streams
/// to logical domains.
class hStreams_LogDomain
{
    /// @brief ID of the logical domain. This is what the user enumerates the logical
    ///     domains by.
    const HSTR_LOG_DOM id_;
    /// @brief The physical domain this logical domain is contained in
    hStreams_PhysDomain *const phys_dom_;
    typedef std::vector<hStreams_LogStream *> LogStreamsContainer;
    /// @brief "Links" to all the logical streams in this physical domain
    LogStreamsContainer log_streams_;
    /// @brief A CPU mask in which the logical streams created in this logical domain
    ///     must be contained. It must fit within the owning physical domain's max cpu mask.
    const hStreams_CPUMask cpu_mask_;
public:
    hStreams_LogDomain(HSTR_LOG_DOM id, hStreams_CPUMask const &cpu_mask, hStreams_PhysDomain &phys_dom);
    /// @brief A helper function to make a lookup of a logical stream by its CPU mask.
    ///
    /// This might be better suited to be located in the logical stream store, i.e.
    /// \c hStreams_LogStreamCollection
    hStreams_LogStream *lookupLogStreamByCPUMask(hStreams_CPUMask const &mask, HSTR_OVERLAP_TYPE *out_overlap) const;
    /// @brief Get a copy of the logical domain's CPU mask
    hStreams_CPUMask getCPUMask() const;
    /// @brief Write out the IDs of logical streams to a vector
    /// @sa hStreams_GetLogStreamIDList
    ///
    /// This might be better suited to be located in the logical stream store, i.e.
    /// \c hStreams_LogStreamCollection
    void getLogStreamIDs(HSTR_LOG_STR *ptr, uint32_t num, uint32_t *num_written, uint32_t *num_present) const;
    /// @brief Get the number of logical streams in this logical domain
    /// @sa hStreams_GetNumLogStreams
    ///
    /// This might be better suited to be located in the logical stream store, i.e.
    /// \c hStreams_LogStreamCollection
    uint32_t getNumLogStreams() const;
    /// @brief Get the CPU mask that is occupied by any stream in this logical domain
    ///
    /// Basically, an \c OR of all streams in this logical domain
    hStreams_CPUMask getOccupiedStreamCPUMask() const;
    /// @brief Close all streams in this logical domain
    /// @note This calls \c delete on pointers to each of the logical streams it knows of
    void destroyAllStreams();
    /// @brief A hook for notifying the logical domain that a new logical stream has been created in it
    void delLogStreamMapping(hStreams_LogStream &log_str);
    /// @brief A hook for notifying the logical domain that a logical stream has been deleted from it
    void addLogStreamMapping(hStreams_LogStream &log_str);
    ~hStreams_LogDomain();
    HSTR_LOG_DOM id() const
    {
        return id_;
    }
    /// @brief Get the owning physical domain
    hStreams_PhysDomain &getPhysDomain() const
    {
        return *phys_dom_;
    }
private:
    // assignment operator is prohibited
    hStreams_LogDomain &operator=(hStreams_LogDomain const &other);
};

#endif /* HSTREAMS_LOGDOMAIN_H */
