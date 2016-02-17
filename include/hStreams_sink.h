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

#ifndef __HSTR_SINK_H__

#define __HSTR_SINK_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "hStreams_version.h"
#include "hStreams_types.h"
#include "hStreams_common.h"

#define HSTREAMSNATIVELIBEXPORT_VISIBILITY "default"

#ifdef __cplusplus
#define HSTREAMSNATIVELIBEXPORT \
    extern "C" __attribute__ ((visibility(HSTREAMSNATIVELIBEXPORT_VISIBILITY)))
#else
#define HSTREAMSNATIVELIBEXPORT \
    __attribute__ ((visibility(HSTREAMSNATIVELIBEXPORT_VISIBILITY)))
#endif

#ifndef _WIN32
#define HSTREAMS_EXPORT HSTREAMSNATIVELIBEXPORT
#else
#define HSTREAMS_EXPORT extern "C" __declspec(dllexport)
#endif

#endif //DOXYGEN_SHOULD_SKIP_THIS

#endif
