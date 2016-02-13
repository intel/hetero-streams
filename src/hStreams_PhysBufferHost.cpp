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
#include <stdlib.h>
#include <stdio.h>

#include "hStreams_PhysBufferHost.h"

hStreams_PhysBufferHost::hStreams_PhysBufferHost(hStreams_LogBuffer const &log_buf, HSTR_COIBUFFER coi_buf, std::unique_ptr<void, void(*)(void *)> data_ptr, uint64_t padding)
    : hStreams_PhysBuffer(log_buf, coi_buf, (uint64_t)data_ptr.get(), padding), data_ptr_(std::move(data_ptr))
{
}

hStreams_PhysBufferHost::~hStreams_PhysBufferHost()
{
}

