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

#include "hStreams_internal_vars_common.h"
#include "hStreams_helpers_common.h"
#include "hStreams_common.h"

#if HSTR_VERSION_MICRO
const char *version_string = hStreams_stringer(HSTR_VERSION_MAJOR) "."
                             hStreams_stringer(HSTR_VERSION_MINOR) "."
                             hStreams_stringer(HSTR_VERSION_MICRO);
#else
const char *version_string = hStreams_stringer(HSTR_VERSION_MAJOR) "."
                             hStreams_stringer(HSTR_VERSION_MINOR);
#endif

HSTR_OPTIONS global_options = HSTR_OPTIONS_INITIAL_VALUES;
hStreams_RW_Lock global_options_lock;

namespace globals
{

HSTR_LOG_LEVEL logging_level = HSTR_LOG_LEVEL_ERROR;
uint64_t logging_bitmask = (uint64_t) - 1;
HSTR_PHYS_DOM logging_myphysdom = HSTR_SRC_PHYS_DOMAIN;
HSTR_MKL_INTERFACE mkl_interface = HSTR_MKL_LP64;

} // namespace globals
