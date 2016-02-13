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

#ifndef HSTREAMS_INTERNAL_VARS_COMMON_H
#define HSTREAMS_INTERNAL_VARS_COMMON_H

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdlib.h> // for exit()

#include "hStreams_locks.h"
#include "hStreams_common.h"

extern const char *library_version_string;


namespace globals
{

#ifdef _WIN32
extern HMODULE hstreams_source_dll_handle;
#endif

extern HSTR_LOG_LEVEL logging_level;
extern uint64_t logging_bitmask;
extern HSTR_PHYS_DOM logging_myphysdom;
extern HSTR_MKL_INTERFACE mkl_interface;

} // namespace globals

#endif /* HSTREAMS_INTERNAL_VARS_COMMON_H */
