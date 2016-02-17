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

#ifndef HSTREAMS_INTERNAL_TYPES_COMMON_H
#define HSTREAMS_INTERNAL_TYPES_COMMON_H

#ifdef _WIN32
#include <Windows.h>
#endif

#include <hStreams_types.h>

struct hStreams_InitSinkData {
    uint32_t phys_domain_id;
    HSTR_LOG_LEVEL logging_level;
    uint64_t logging_bitmask;
    HSTR_PHYS_DOM logging_myphysdom;
    HSTR_MKL_INTERFACE mkl_interface;
};

namespace LIB_HANDLER
{
#ifndef _WIN32
typedef void *handle_t;
#else
typedef HMODULE handle_t;
#endif
}

#endif /* HSTREAMS_INTERNAL_TYPES_COMMON_H */

