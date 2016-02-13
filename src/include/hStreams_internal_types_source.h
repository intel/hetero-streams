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

#ifndef HSTREAMS_INTERNAL_TYPES_SOURCE_H
#define HSTREAMS_INTERNAL_TYPES_SOURCE_H

enum HSTR_STATE {
    // HSTR_STATE_UNINITIALIZED means that hStreams_Init() has not been called yet,
    // hStreams_Init() was called and then hStreams_Fini() was called and
    // hStreams_Fini() completed.
    HSTR_STATE_UNINITIALIZED = 0,
    // HSTR_STATE_INITIALIZING means that exactly one thread is executing the
    // hStreams_Init() code.
    HSTR_STATE_INITIALIZING  = 1,
    // HSTR_STATE_INITIALIZED means that hStreams_Init() succeeded, and the hStreams
    // library is open for all calls.
    HSTR_STATE_INITIALIZED   = 2,
    // HSTR_STATE_FINALIZING means that exactly one thread is executing the
    // hStreams_Fini() code, and it is taking down the internal data structures.
    // After hStreams_Fini() completes, the next state is
    // HSTR_STATE_UNINITIALIZED
    HSTR_STATE_FINALIZING    = 3,
};

typedef hStreams_AtomicEnum<HSTR_STATE> hStreams_Atomic_HSTR_STATE;


#endif /* HSTREAMS_INTERNAL_TYPES_SOURCE_H */
