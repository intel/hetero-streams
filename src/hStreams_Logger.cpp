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

#include <iomanip>
#include <fstream>

#include "hStreams_Logger.h"
#include "hStreams_internal.h"
#include "hStreams_internal_vars_common.h"
#include "hStreams_helpers_common.h"

Logger::Logger(const char *file_name, int line_number, const char *function_name, int exit_code) :
    null_os_(0),
    file_name_(file_name),
    line_number_(line_number),
    function_name_(function_name),
    exit_code_(exit_code)
{
}

Logger::~Logger()
{
    if (!oss_.str().empty()) {

        oss_ << " (" << file_name_ << ":"
             << line_number_ << ")" << std::endl;

        *output_stream_ << oss_.str() << std::flush;
    }

    if (exit_code_) {
        hStreams_GetOptions__hStreams_FatalError()(exit_code_);
    }
}

std::ostream &Logger::get(HSTR_LOG_LEVEL log_level, HSTR_INFO_TYPE info_type)
{
    using globals::logging_myphysdom;

    if (!shouldLog(log_level, info_type)) {
        return null_os_;
    }

    if (log_level <= HSTR_LOG_LEVEL_WARN) {
        output_stream_ = &std::cerr;
    } else {
        output_stream_ = &std::cout;
    }

    //Append start of message
    oss_ << std::right
         << std::setw(10) << getTimestamp() << " "
         << std::setw(3) << logging_myphysdom << " "
         << std::left
         << std::setw(11) << getProcessIdAsString()
         << std::setw(13) << getThreadIdAsString()
         << std::setw(8) << getLogLevelName(log_level)
         << std::setw(9) << getInfoTypeName(info_type)
         << std::setw(0) << std::right;

    return oss_;
}

bool Logger::shouldLog(HSTR_LOG_LEVEL log_level, HSTR_INFO_TYPE info_type)
{
    using globals::logging_level;
    using globals::logging_bitmask;
    return log_level <= logging_level && (info_type & logging_bitmask);
}

static const char *hStreamsInfoTypeNames[] = {
    "DEPR",
    "DEPR",
    "DEPR",
    "DEPR",
    "DEPR",
    "DEPR",
    "TRACE",
    "INVOKE",
    "ALLOC",
    "SYNC",
    "MISC"
};


const char *Logger::getInfoTypeName(HSTR_INFO_TYPE info_type)
{
    int index = 0;
    while (info_type > 0) {
        info_type >>= 1;
        ++index;
    }

    return hStreamsInfoTypeNames[index];
}

static const char *hStreamsLogLevelNames[] = {
    "NOLOG",
    "FATAL_ERROR",
    "ERROR",
    "WARN",
    "LOG",
    "DEBUG1",
    "DEBUG2",
    "DEBUG3",
    "DEBUG4"
};

const char *Logger::getLogLevelName(HSTR_LOG_LEVEL log_level)
{
    return hStreamsLogLevelNames[log_level];
}


