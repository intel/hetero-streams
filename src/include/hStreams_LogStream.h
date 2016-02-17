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

#ifndef HSTREAMS_LOGSTREAM_H
#define HSTREAMS_LOGSTREAM_H

#include "hStreams_types.h"
#include "hStreams_LogDomain.h"
#include "hStreams_PhysStream.h"

#include <vector>

/// @brief A class which represents an individual logical stream
///
/// A logical stream is an entity enumerable by the user. A logical stream does not
/// own any resources itself, neither does it perform the actions. A logical stream
/// maps onto a physical stream, generalized by the \c hStreams_PhysStream class.
/// There may be a many to one mapping of logical to physical streams if the logical
/// streams belong in the same logical domain and have the same CPU mask.
///
/// Implementation-wise, a logical stream _attaches_ to a physical stream, thus
/// increasing the physical stream's refcount.
class hStreams_LogStream
{
    /// @brief ID of the logical stream. This is what the user enumerates the logical
    ///     streams by.
    const HSTR_LOG_STR id_;
    /// @brief The logical domain this logical stream is contained in
    hStreams_LogDomain *const log_dom_;
    /// @brief The physical stream this logical stream maps to
    hStreams_PhysStream *const phys_stream_;
    /// @brief The CPU mask this logical stream occupies
    ///
    /// @note This entry is superfluous and the usage of it could well be replaced
    ///     by calls to \c phys_stream_->getCPUMask()
    const hStreams_CPUMask cpu_mask_;
public:
    /// @param[in] id ID of the logical stream.
    /// @param[in] cpu_mask CPU mask this logical stream shall occupy
    /// @param[in] log_dom The logical domain this logical streams belongs to
    /// @param[in] phys_stream The physical stream this logical stream maps onto
    ///
    /// During its construction, the logical stream object automatically attaches to the
    /// physical stream.
    hStreams_LogStream(HSTR_LOG_STR id, hStreams_CPUMask const &cpu_mask, hStreams_LogDomain &log_dom, hStreams_PhysStream &phys_stream);
    ///
    /// During its destruction, the logical stream object automatically attaches to the
    /// physical stream.
    ~hStreams_LogStream();
    /// @brief Get the ID of the logical stream
    HSTR_LOG_STR id() const;
    /// @brief Get a copy of the CPU mask this stream occupies
    hStreams_CPUMask getCPUMask() const;
    /// @brief Get a "link" to the logical domain this stream belongs to
    hStreams_LogDomain &getLogDomain();
    /// @brief Get a "link" to the underlying physical stream
    hStreams_PhysStream &getPhysStream();
    /// @brief Get all the events used thus fat in this stream.
    void getAllEvents(std::vector<HSTR_EVENT> &events);
private:
    // assingment prohibited
    hStreams_LogStream &operator=(hStreams_LogStream const &other);
};


#endif /* HSTREAMS_LOGSTREAM_H */
