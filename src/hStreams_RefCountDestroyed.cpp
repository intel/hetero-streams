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

#include "hStreams_RefCountDestroyed.h"

hStreams_RefCountDestroyed::hStreams_RefCountDestroyed()
    : refcount_(1)
{}

hStreams_RefCountDestroyed::~hStreams_RefCountDestroyed()
{}

void hStreams_RefCountDestroyed::attach()
{
    // do it on atomics, I guess?
    refcount_++;
}

void hStreams_RefCountDestroyed::detach()
{
    // do it on atomics, I guess?
    if (0 == --refcount_) {
        delete this;
    }
}
