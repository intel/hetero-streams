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

#include "hStreams_LogStream.h"

HSTR_LOG_STR hStreams_LogStream::id() const
{
    return id_;
}

hStreams_CPUMask hStreams_LogStream::getCPUMask() const
{
    return cpu_mask_;
}

hStreams_LogDomain &hStreams_LogStream::getLogDomain()
{
    return *log_dom_;
}

hStreams_PhysStream &hStreams_LogStream::getPhysStream()
{
    return *phys_stream_;
}

hStreams_LogStream::hStreams_LogStream(HSTR_LOG_STR id, hStreams_CPUMask const &my_cpu_mask, hStreams_LogDomain &log_dom, hStreams_PhysStream &phys_stream)
    : id_(id), cpu_mask_(my_cpu_mask), log_dom_(&log_dom), phys_stream_(&phys_stream)
{
    phys_stream_->attach();
}

hStreams_LogStream::~hStreams_LogStream()
{
    phys_stream_->detach();
}

void hStreams_LogStream::getAllEvents(std::vector<HSTR_EVENT> &events)
{
    phys_stream_->getAllEvents(events);
}
