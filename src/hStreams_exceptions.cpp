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
        Logger(e.file_name(), e.line_number(), e.function_name(), 0).get(
            HSTR_LOG_LEVEL_ERROR, e.info_type())
                << hStreams_ResultGetName(e.error_code())
                << ": "
                << e.what();

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

hStreams_exception::hStreams_exception(
    const char *file_name,
    int line_number,
    const char *function_name,
    HSTR_RESULT error_code,
    std::string const &msg,
    HSTR_INFO_TYPE info_type
) :
    error_code_(error_code),
    msg_(msg),
    file_name_(file_name),
    line_number_(line_number),
    function_name_(function_name),
    info_type_(info_type)
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

const char *hStreams_exception::file_name() const
{
    return file_name_;
}

int hStreams_exception::line_number() const
{
    return line_number_;
}

const char *hStreams_exception::function_name() const
{
    return function_name_;
}

HSTR_INFO_TYPE hStreams_exception::info_type() const
{
    return info_type_;
}
