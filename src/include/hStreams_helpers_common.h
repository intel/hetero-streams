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

#ifndef HSTREAMS_HELPERS_COMMON_H
#define HSTREAMS_HELPERS_COMMON_H

#include <string>
#include <stdint.h>

#include "hStreams_internal_types_common.h"

/*****************************************************************************************
   Explanation of the preprocessor stringification code:
   1. Define the macro 'hStreams_stringer(ARG)' as the undefined 'hStreams_stringer1(ARG)'
      This is  esentially a forward declaration, because the macro hStreams_stringer() is
      not being used right now, just defined.
****************************************************************************************/
#define hStreams_stringer(ARG) hStreams_stringer1(ARG)
/*****************************************************************************************
   Explanation of the preprocessor stringification code (continued):
   2. Define the macro 'hStreams_stringer1(ARG)' as the C preprocessor operator
      stringizing the argument.  For example, hStreams_stringer1(XXX) is "XXX". (Do a
      google search on stringification and token pasting for details on this.)

   Usage example:

   Stringifying a numeric literal:
     hStreams_stringer(1234) -> "1234"
        NOTE: this would work as well without using the indirection, i.e. by using
              hStreams_stringer1() directly.

   Stringifying the result of a preprocessor macro:
     hStreams_stringer(__LINE__) -> "34" (at the time of writing)
        NOTE: simple token pasting (without indirection, e.g. by using hStreams_stringer1)
              would place the literal inside quotes, i.e. "__LINE__".
              Using additional level of indirection results in __LINE__ being expanded
              to its proper value.

   Stringifying a compiler command-line-defined preprocessor definition:
     gcc -DFOO=BAR
     hStreams_stringer(FOO) -> "BAR"

****************************************************************************************/
#define hStreams_stringer1(ARG) #ARG

/// @brief Returns number of miliseconds since linux epoch
/// (For windows as well).
uint64_t getTimestamp();

/// @brief Returns process id coded as hex.
std::string getProcessIdAsString();

/// @brief Returns thread id coded as hex.
std::string getThreadIdAsString();

class hStreams_LibLoader
{
public:
    static void load(std::string const &full_path, LIB_HANDLER::handle_t &handle);
    static void unload_nothrow(LIB_HANDLER::handle_t const &handle);
    static uint64_t fetchFunctionAddress(LIB_HANDLER::handle_t lib_handle, std::string const &func_name);
    static uint64_t fetchFunctionAddress_nothrow(LIB_HANDLER::handle_t lib_handle, std::string const &func_name);
    static uint64_t fetchGlobalFunctionAddress_nothrow(std::string const &func_name);
    static uint64_t fetchExecFunctionAddress_nothrow(std::string const &func_name);

#ifndef _WIN32
    static uint64_t fetchVersionedFunctionAddress(LIB_HANDLER::handle_t lib_handle, std::string const &func_name, std::string const &version);
#endif // _WIN32
};

#endif /* HSTREAMS_HELPERS_COMMON_H */
