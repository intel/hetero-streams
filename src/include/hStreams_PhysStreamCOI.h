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

#ifndef HSTREAMS_PHYSSTREAMCOI_H
#define HSTREAMS_PHYSSTREAMCOI_H

#include <map>

#include "hStreams_PhysStream.h"
#include "hStreams_COIWrapper.h"

class hStreams_LogDomain;

// Implementation of a physical stream over COI, calls COIPipelineDestroy on
// the pipeline it has been created with upon the object's destruction
class hStreams_PhysStreamCOI : public hStreams_PhysStream
{
    const HSTR_COIFUNCTION thunk_func_;
    const HSTR_COIPIPELINE coi_pipeline_;
public:
    /// @param log_dom  The logical domain this physical stream is created in
    /// @param cpu_mask The CPU mask of the stream
    /// @param coi_pipeline The pre-created COI pipeline in which to run actions
    /// @param thunk_func   A COI handle to the sink-side thunk function
    /// @param fetch_addr_func     A COI handle to the sink-side function which
    ///             performs a lookup (dlsym-like) of a function address by the
    ///             function's name
    hStreams_PhysStreamCOI(
        hStreams_LogDomain &log_dom,
        hStreams_CPUMask const &cpu_mask,
        HSTR_COIPIPELINE coi_pipeline,
        HSTR_COIFUNCTION thunk_func
    );
    /// @note The destructor calls COIPipelineDestroy on the pipeline the stream has
    ///         been created with.
    ~hStreams_PhysStreamCOI();
private:
    /// @brief Implementation of the enqueue a "function execution" action. Called by
    ///         the physical stream's interface (\c hStreams_PhysStream::enqueueFunction())
    HSTR_RESULT impl_enqueueFunction(
        std::vector<uint64_t> &args,
        std::vector<HSTR_EVENT> &input_deps,
        void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
    );
};

#endif /* HSTREAMS_PHYSSTREAMCOI_H */
