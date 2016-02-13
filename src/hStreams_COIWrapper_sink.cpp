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

#include <iostream>
#include <cstdlib>

#include "hStreams_COIWrapper_sink.h"
#include "hStreams_helpers_common.h"

#define INITIALIZE_STATIC_HANDLE(func_name)\
    hStreams_COIWrapper_sink::func_name##_handler_t hStreams_COIWrapper_sink::func_name = NULL

#define FETCH_COI_FUNCTION_ADDRESS(func_name)\
    func_name = (func_name##_handler_t) hStreams_LibLoader::fetchFunctionAddress(COI_handler, #func_name)

hStreams_COIWrapper_sink::hStreams_COIWrapper_sink()
{
    try {
        //Open COI library
        LIB_HANDLER::handle_t COI_handler;
        hStreams_LibLoader::load("libcoi_device.so.0", COI_handler);

        //Fetch COI functions' symbols
        FETCH_COI_FUNCTION_ADDRESS(COIProcessWaitForShutdown);
        FETCH_COI_FUNCTION_ADDRESS(COIPipelineStartExecutingRunFunctions);
    } catch (...) {
        std::cerr << "Error while loading libcoi_device.so.0 shared library." << std::endl;
        exit(1);
    }
}

INITIALIZE_STATIC_HANDLE(COIProcessWaitForShutdown);
INITIALIZE_STATIC_HANDLE(COIPipelineStartExecutingRunFunctions);
