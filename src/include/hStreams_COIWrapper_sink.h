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

#ifndef HSTREAMS_COI_WRAPPER_SINK_H
#define HSTREAMS_COI_WRAPPER_SINK_H

#include "hStreams_COIWrapper_types.h"
#include "hStreams_exceptions.h"

/// @brief Class load COI library on device and store handles to COI methods
class hStreams_COIWrapper_sink
{
private:
    //COI function handlers' types
    typedef HSTR_COIRESULT(*COIProcessWaitForShutdown_handler_t)();
    typedef HSTR_COIRESULT(*COIPipelineStartExecutingRunFunctions_handler_t)();

public:
    hStreams_COIWrapper_sink();

    //COI function handlers
    static COIProcessWaitForShutdown_handler_t                      COIProcessWaitForShutdown;
    static COIPipelineStartExecutingRunFunctions_handler_t          COIPipelineStartExecutingRunFunctions;
};

#endif

