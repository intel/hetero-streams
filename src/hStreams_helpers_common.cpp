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

#include "hStreams_types.h"
#include "hStreams_exceptions.h"
#include "hStreams_helpers_common.h"
#include "hStreams_internal_vars_source.h"
#include "hStreams_Logger.h"

#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <Windows.h>
#else
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#endif

namespace
{
std::string uint64_tToHexString(uint64_t number)
{
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << number;
    return ss.str();
}
} // anonymous namespace


#ifdef _WIN32
uint64_t getTimestamp()
{
    union {
        FILETIME t;
        int64_t i;
    } time_u;
    GetSystemTimeAsFileTime(&time_u.t);
    const int64_t win_tick_to_ms = 10 * 1000; // Windows tick is 100 nanoseconds
    const int64_t win_unix_adjust = 11644473600LL; // Windows epoch starts 1601-01-01T00:00:00Z
    return (uint64_t)(time_u.i / win_tick_to_ms - win_unix_adjust);
}
std::string getProcessIdAsString()
{
    return uint64_tToHexString(GetCurrentProcessId());
}
std::string getThreadIdAsString()
{
    return uint64_tToHexString(GetCurrentThreadId());
}
#else // _WIN32
uint64_t getTimestamp()
{
    timeval tv;
    gettimeofday(&tv, NULL);

    return (uint64_t)(tv.tv_sec) * 1000 +
           (uint64_t)(tv.tv_usec) / 1000;
}
std::string getProcessIdAsString()
{
    return uint64_tToHexString(getpid());
}

std::string getThreadIdAsString()
{
    return uint64_tToHexString(syscall(SYS_gettid));
}
#endif // _WIN32

#ifndef _WIN32

void hStreams_LibLoader::load(std::string const &full_path, LIB_HANDLER::handle_t &handle)
{
    handle = dlopen(full_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
        char *err = dlerror();
        throw hStreams_exception(HSTR_RESULT_BAD_NAME, (err != NULL ? err : "Cannot load library " + full_path));
    }
}

void hStreams_LibLoader::unload_nothrow(LIB_HANDLER::handle_t const &handle)
{
    if (dlclose(handle) != 0) {
        char *err = dlerror();
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "A problem encountered while unloading library: "
                                       << (err == NULL ? "" : err);
    }
}

uint64_t hStreams_LibLoader::fetchFunctionAddress(LIB_HANDLER::handle_t lib_handle, std::string const &func_name)
{
    uint64_t address = NULL;
    address = (uint64_t) dlsym(lib_handle, func_name.c_str());
    if (address == NULL) {
        char *err = dlerror();
        throw hStreams_exception(HSTR_RESULT_BAD_NAME, (err != NULL ? err : "Cannot fetch function address " + func_name));
    }
    return address;
}

uint64_t hStreams_LibLoader::fetchVersionedFunctionAddress(LIB_HANDLER::handle_t lib_handle, std::string const &func_name, std::string const &version)
{
    uint64_t address = NULL;
    address = (uint64_t) dlvsym(lib_handle, func_name.c_str(), version.c_str());

    if (address == NULL) {
        char *err = dlerror();
        throw hStreams_exception(HSTR_RESULT_BAD_NAME, (err != NULL ? err : "Cannot fetch function address " + func_name));
    }
    return address;
}

uint64_t hStreams_LibLoader::fetchFunctionAddress_nothrow(LIB_HANDLER::handle_t lib_handle, std::string const &func_name)
{
    return (uint64_t) dlsym(lib_handle, func_name.c_str());
}

uint64_t hStreams_LibLoader::fetchExecFunctionAddress_nothrow(std::string const &func_name)
{
    LIB_HANDLER::handle_t handle = dlopen(NULL, RTLD_LAZY);

    if (handle != NULL) {
        return (uint64_t) dlsym(handle, func_name.c_str());
    }
    return 0;
}

uint64_t hStreams_LibLoader::fetchGlobalFunctionAddress_nothrow(std::string const &func_name)
{
    return (uint64_t) dlsym(RTLD_DEFAULT, func_name.c_str());
}

#else // _WIN32
void hStreams_LibLoader::load(std::string const &full_path, LIB_HANDLER::handle_t &handle)
{
    handle = LoadLibrary(full_path.c_str());

    if (handle == NULL) {
        throw hStreams_exception(HSTR_RESULT_BAD_NAME, "Cannot load library " + full_path);
    }
}

void hStreams_LibLoader::unload_nothrow(LIB_HANDLER::handle_t const &handle)
{
    if (FreeLibrary(handle) == 0) {
        HSTR_WARN(HSTR_INFO_TYPE_MISC) << "A problem encountered while unloading library: "
                                       << GetLastError();
    }
}

uint64_t hStreams_LibLoader::fetchFunctionAddress(LIB_HANDLER::handle_t lib_handle, std::string const &func_name)
{
    uint64_t address = NULL;
    address = (uint64_t) GetProcAddress(lib_handle, func_name.c_str());
    if (address == NULL) {
        throw hStreams_exception(HSTR_RESULT_BAD_NAME, "Cannot fetch function address " + func_name);
    }
    return address;
}

uint64_t hStreams_LibLoader::fetchFunctionAddress_nothrow(LIB_HANDLER::handle_t lib_handle, std::string const &func_name)
{
    return (uint64_t) GetProcAddress(lib_handle, func_name.c_str());
}

uint64_t hStreams_LibLoader::fetchExecFunctionAddress_nothrow(std::string const &func_name)
{
    return (uint64_t) GetProcAddress(GetModuleHandle(NULL), func_name.c_str());
}

uint64_t hStreams_LibLoader::fetchGlobalFunctionAddress_nothrow(std::string const &func_name)
{
    return (uint64_t) GetProcAddress(globals::hstreams_source_dll_handle, func_name.c_str());
}

#endif // _WIN32

