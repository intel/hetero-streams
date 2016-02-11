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

#ifndef HSTREAMS_PHYSBUFFERHOST_H
#define HSTREAMS_PHYSBUFFERHOST_H

#include <memory>

#include "hStreams_PhysBuffer.h"

class hStreams_LogBuffer;
class hStreams_PhysBufferHost : public hStreams_PhysBuffer
{
    /// @brief The automatically-deleted data we allocated for the host-side buffer
    std::unique_ptr<void, void(*)(void *)> data_ptr_;
public:
    /// @brief The constructor which sets up the internals
    ///
    /// @note The allocation of the memory to create a \c HSTR_COIBUFFER from must
    ///     happen outside of this class. During its construction, the "host"
    ///     physical buffer will take ownership of the memory and shall free it
    ///     during its destruction.
    ///
    /// @note Recognizing the necessity for aligned allocations, the unique
    ///     pointer is specified with a deleter which conforms to the \c free()
    ///     / \c _aligned_free() functions. Alternatively, one should remember
    ///     that one can achieve the proper alignment through allocting more
    ///     memory and calculating proper offset. We might want
    ///     to it that way eventually.
    hStreams_PhysBufferHost(hStreams_LogBuffer const &log_buf, HSTR_COIBUFFER coi_buf, std::unique_ptr<void, void(*)(void *)> data_ptr, uint64_t padding);
    ~hStreams_PhysBufferHost();
};

#endif /* HSTREAMS_PHYSBUFFERHOST_H */
