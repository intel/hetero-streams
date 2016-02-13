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

#ifndef __HSTR_COMMON_H__
#define __HSTR_COMMON_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "hStreams_version.h"
#include "hStreams_types.h"

#endif //DOXYGEN_SHOULD_SKIP_THIS

// Defines that pertain to dependence enforcement for waits
#define HSTR_WAIT_CONTROL 0 // Control dependence on all actions
#define HSTR_WAIT_NONE -1   // No control or data dependences

// The maximum size of any user-defined function value
#define HSTR_RETURN_SIZE_LIMIT 64

// The number of arguments that hStreams_EnqueueCompute supports, that are a combination of
// scalars and heap argument for this version of hStreams:
#define HSTR_ARGS_IMPLEMENTED 19

// A special value of the physical domain index is -1, for the source domain
#define HSTR_SRC_PHYS_DOMAIN -1

// A special value of the logical domain ID, for the source domain
#define HSTR_SRC_LOG_DOMAIN 0

// A special value of the timeout duration is -1, for infinite
#define HSTR_TIME_INFINITE -1

// Maximum sink-side function name size
#define HSTR_MAX_FUNC_NAME_SIZE 80

// COI pipeline function miscellaneous data size
#define HSTR_MISC_DATA_SIZE 4096

// Maximum number of sink-side function arguments
#define HSTR_ARGS_SUPPORTED (HSTR_MISC_DATA_SIZE-HSTR_MAX_FUNC_NAME_SIZE)/sizeof(uint64_t)

#ifdef _WIN32
#       if defined _EXPORT_SYMBOLS
#          define DllAccess  __declspec(dllexport)
#       elif defined _IMPORT_SYMBOLS
#          define DllAccess  __declspec(dllimport)
#       else
#          define DllAccess  /* nothing */
#       endif
#else
#       define DllAccess  /* nothing */
#endif

#endif // __HSTR_COMMON_H__
