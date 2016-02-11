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

#include "hStreams_exceptions.h"
#include "hStreams_common.h"
#include "hStreams_internal.h"
#include "hStreams_Logger.h"

#include <new>

HSTR_RESULT hStreams_handle_exception()
{
    try {
        throw; // re-throw the active exception so that we can analyze it
    } catch (hStreams_exception const &e) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << e.what();

        return e.error_code();
    } catch (std::bad_alloc const &e) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Internal allocation failed: " << e.what();

        return HSTR_RESULT_INTERNAL_ERROR;
    } catch (std::exception const &e) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << e.what();

        return HSTR_RESULT_INTERNAL_ERROR;
    } catch (...) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "unknown internal error";

        return HSTR_RESULT_INTERNAL_ERROR;
    }
}

hStreams_exception::hStreams_exception(HSTR_RESULT error_code)
    : error_code_(error_code), msg_(hStreams_ResultGetName(error_code))
{

}

hStreams_exception::hStreams_exception(HSTR_RESULT error_code, std::string const &msg)
    : error_code_(error_code), msg_(msg)
{

}

const char *hStreams_exception::what() const throw()
{
    return msg_.c_str();
}

HSTR_RESULT hStreams_exception::error_code() const
{
    return error_code_;
}
