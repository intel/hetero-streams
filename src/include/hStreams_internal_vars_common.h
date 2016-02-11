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

#include <stdlib.h> // for exit()

#include "hStreams_locks.h"
#include "hStreams_common.h"

extern const char *version_string;

#define HSTR_DEFAULT_DEVICES 32
#define HSTR_TIME_OUT_MS -1 // -1 is indefinite; could be 500000
#define HSTR_OPTIONS_INITIAL_VALUES  { \
        NULL,         \
        exit,                          \
        0,                             \
        HSTR_KMP_AFFINITY_BALANCED,    \
        HSTR_DEP_POLICY_BUFFERS,       \
        HSTR_DEFAULT_DEVICES,          \
        HSTR_OPENMP_PRE_SETUP,         \
        HSTR_TIME_OUT_MS,              \
        0,                             \
        NULL,                          \
        NULL,                          \
        0,                             \
        NULL                           \
    }

extern HSTR_OPTIONS global_options;
extern hStreams_RW_Lock global_options_lock;

namespace globals
{

extern HSTR_LOG_LEVEL logging_level;
extern uint64_t logging_bitmask;
extern HSTR_PHYS_DOM logging_myphysdom;
extern HSTR_MKL_INTERFACE mkl_interface;

} // namespace globals

#endif /* HSTREAMS_INTERNAL_VARS_COMMON_H */
