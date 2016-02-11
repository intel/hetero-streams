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

#ifndef HSTREAMS_PHYSSTREAMHOST_H
#define HSTREAMS_PHYSSTREAMHOST_H

#include <map>
#include <vector>
#include <memory>

#include "hStreams_PhysStream.h"
#include "hStreams_HostSideSinkWorker.h"
#include "hStreams_COIWrapper.h"

class hStreams_LogDomain;

/// @brief A custom implementation of physical stream on the host
///
/// COI's POR does not include creating pipelines on the host/source
/// so we had to roll our own.
class hStreams_PhysStreamHost : public hStreams_PhysStream
{
public:
    hStreams_PhysStreamHost(
        hStreams_LogDomain &log_dom,
        hStreams_CPUMask const &cpu_mask
    );
    ~hStreams_PhysStreamHost();
private:
    std::unique_ptr<hStreams_HostSideSinkWorker> hostSinkWorker_;

    HSTR_RESULT impl_enqueueFunction(
        std::vector<uint64_t> &args,
        std::vector<HSTR_EVENT> &input_deps,
        void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
    );
    uint64_t impl_fetchSinkFunctionAddress(std::string const &func_name);
};


#endif /* HSTREAMS_PHYSSTREAMHOST_H */
