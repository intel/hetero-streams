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

#include "hStreams_COIWrapper.h"
#include "hStreams_helpers_common.h"

#include <iostream>
#include <cstdlib>

#ifndef _WIN32
#include "dlfcn.h"
#endif

// Both below macros assume existence of handler to COI function in hStreams_COIWrapper class which:
// - have the same name as that function
// - is of type named same as that function with _handler_t postfix

// Define function handle with is static member of hStreams_COIWrapper class
#define DEFINE_STATIC_HANDLE(func_name)\
    hStreams_COIWrapper::func_name##_handler_t hStreams_COIWrapper::func_name = NULL

// Assign COI function address to its handler
#define FETCH_COI_FUNCTION_ADDRESS(func_name)\
    hStreams_COIWrapper::func_name = (hStreams_COIWrapper::func_name##_handler_t) hStreams_LibLoader::fetchFunctionAddress(COI_handler, #func_name)

//Define static variables
DEFINE_STATIC_HANDLE(COIEngineGetInfo);
DEFINE_STATIC_HANDLE(COIEngineGetCount);
DEFINE_STATIC_HANDLE(COIEngineGetHandle);

DEFINE_STATIC_HANDLE(COIEventWait);
DEFINE_STATIC_HANDLE(COIEventRegisterUserEvent);
DEFINE_STATIC_HANDLE(COIEventSignalUserEvent);

DEFINE_STATIC_HANDLE(COIProcessCreateFromFile);
DEFINE_STATIC_HANDLE(COIProcessCreateFromMemory);
DEFINE_STATIC_HANDLE(COIProcessDestroy);
DEFINE_STATIC_HANDLE(COIProcessGetFunctionHandles);
DEFINE_STATIC_HANDLE(COIProcessLoadLibraryFromFile);
DEFINE_STATIC_HANDLE(COIProcessUnloadLibrary);

DEFINE_STATIC_HANDLE(COIBufferCreate);
DEFINE_STATIC_HANDLE(COIBufferCreateFromMemory);
DEFINE_STATIC_HANDLE(COIBufferDestroy);
DEFINE_STATIC_HANDLE(COIBufferGetSinkAddress);
DEFINE_STATIC_HANDLE(COIBufferWrite);
DEFINE_STATIC_HANDLE(COIBufferCopy);
DEFINE_STATIC_HANDLE(COIBufferSetState);
DEFINE_STATIC_HANDLE(COIBufferAddRefcnt);

DEFINE_STATIC_HANDLE(COIPipelineCreate);
DEFINE_STATIC_HANDLE(COIPipelineDestroy);
DEFINE_STATIC_HANDLE(COIPipelineRunFunction);
DEFINE_STATIC_HANDLE(COIResultGetName);


hStreams_COIWrapper::hStreams_COIWrapper()
{
    // Do once
    // Locking mechanism is not needed here due function is call in critical section
    static bool initialized = false;

    if (initialized) {
        return;
    }
    initialized = true;

    try {
        //Open COI library
        LIB_HANDLER::handle_t COI_handler;
        hStreams_LibLoader::load(
#ifdef _WIN32
            "coi_host.dll",
#else
            "libcoi_host.so.0",
#endif
            COI_handler);

        //Fetch COI functions' symbols
        FETCH_COI_FUNCTION_ADDRESS(COIEngineGetInfo);
        FETCH_COI_FUNCTION_ADDRESS(COIEngineGetCount);
        FETCH_COI_FUNCTION_ADDRESS(COIEngineGetHandle);

        FETCH_COI_FUNCTION_ADDRESS(COIEventWait);
        FETCH_COI_FUNCTION_ADDRESS(COIEventRegisterUserEvent);
        FETCH_COI_FUNCTION_ADDRESS(COIEventSignalUserEvent);

        FETCH_COI_FUNCTION_ADDRESS(COIProcessCreateFromFile);
        FETCH_COI_FUNCTION_ADDRESS(COIProcessCreateFromMemory);
        FETCH_COI_FUNCTION_ADDRESS(COIProcessDestroy);
        FETCH_COI_FUNCTION_ADDRESS(COIProcessGetFunctionHandles);
        FETCH_COI_FUNCTION_ADDRESS(COIProcessUnloadLibrary);

        FETCH_COI_FUNCTION_ADDRESS(COIBufferCreate);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferCreateFromMemory);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferDestroy);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferGetSinkAddress);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferWrite);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferCopy);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferSetState);
        FETCH_COI_FUNCTION_ADDRESS(COIBufferAddRefcnt);

        FETCH_COI_FUNCTION_ADDRESS(COIPipelineCreate);
        FETCH_COI_FUNCTION_ADDRESS(COIPipelineDestroy);
        FETCH_COI_FUNCTION_ADDRESS(COIPipelineRunFunction);
        FETCH_COI_FUNCTION_ADDRESS(COIResultGetName);

        // Break abstraction to be sure that we using right function version
#ifdef _WIN32
        FETCH_COI_FUNCTION_ADDRESS(COIProcessLoadLibraryFromFile);
#else
        hStreams_COIWrapper::COIProcessLoadLibraryFromFile =
            (hStreams_COIWrapper::COIProcessLoadLibraryFromFile_handler_t)
            hStreams_LibLoader::fetchVersionedFunctionAddress(COI_handler, "COIProcessLoadLibraryFromFile", "COI_2.0");
#endif
    } catch (hStreams_exception const &e) {
        std::cerr << "Error while loading shared libraries: " << e.what() << std::endl;
        exit(e.error_code());
    }
}

