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

#ifndef __HSTR_LOGGER_H__

#define __HSTR_LOGGER_H__

#include <fstream>
#include <sstream>
#include <iostream>
#include "hStreams_types.h"
#include "hStreams_internal.h"
#include "hStreams_helpers_common.h"

#define HSTR_GET_LOGGER(log_level, info_type, exit_code) \
    Logger(__FILE__, __LINE__, __FUNCTION__, exit_code).get(log_level, info_type)

#define HSTR_DEBUG1(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_DEBUG1, info_type, 0)

#define HSTR_DEBUG2(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_DEBUG2, info_type, 0)

#define HSTR_DEBUG3(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_DEBUG3, info_type, 0)

#define HSTR_DEBUG4(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_DEBUG4, info_type, 0)

#define HSTR_LOG(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_LOG, info_type, 0)

#define HSTR_WARN(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_WARN, info_type, 0)

#define HSTR_ERROR(info_type) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_ERROR, info_type, 0)

#define HSTR_FATAL_ERROR(info_type, exit_code) \
    HSTR_GET_LOGGER(HSTR_LOG_LEVEL_FATAL_ERROR, info_type, exit_code)

#define HSTR_TRACE_FUN_ENTER() \
    HSTR_DEBUG2(HSTR_INFO_TYPE_TRACE) \
            << __FUNCTION__ \
            << ": entering function."

#define HSTR_TRACE_FUN_ARG(arg)                         \
    HSTR_DEBUG3(HSTR_INFO_TYPE_TRACE)                   \
            << "args[" << hStreams_stringer(arg) << "]: " \
            << arg << "."

#define HSTR_TRACE_FUN_ARG_STR(arg)              \
    HSTR_DEBUG3(HSTR_INFO_TYPE_TRACE)                   \
            << "args[" << hStreams_stringer(arg) << "]: " \
            << (arg == NULL ? "NULL" : arg) << "."

/// @brief Logger class. All logs should be done using this class.
/// Logger format is defined as follows:
/// Timestamp Source ProcessId ThreadId LogLevel InfoType Message (FileName:Line)
/// ,where
///  Timestamp is number of miliseconds since linux epoch,
///  Source is id of physical domain (-1 for host)
///  ProcessId is id of process output as hex value
///  ThreadId is id of thread output as hex value
///  LogLevel is level at which message is logged (see HSTR_LOG_LEVEL)
///  InfoType is info type to which message belongs (see HSTR_INFO_TYPE)
///  Message is message specific for this entry
///  FileName:Line is pair defining where log message was done/

/// @brief This is the main logging utility for hetero-streams
///
/// This class uses RAII semantics of C++ to incrementally construct the log message
/// through the shift operator and output it to the appropriate stream when the
/// instance of the Logger class is destroyed.
///
/// It is meant to be used primarily through the helper macros such as HSTR_WARN()
class Logger
{

private:
    /// @brief Output stringstream in which to accumulate the output
    std::ostringstream oss_;
    /// @brief "Bad" output stream which acts as a sinkhole
    std::ostream null_os_;
    /// @brief The actual stream to output to
    std::ostream *output_stream_;
    /// @brief The name of the file in which the logger was invoked
    const char *file_name_;
    /// @brief The name of the function in which the logger was invoked
    const char *function_name_;
    /// @brief The line number in which the logger was invoked
    int line_number_;
    /// @brief The exit code, if fatal error handler should be called
    int exit_code_;

public:
    /// @brief Constructor, does nothing special
    Logger(const char *file_name, int line_number, const char *function_name, int exit_code);
    /// @brief Destructor, outputs the accumulated stringstream to the output stream
    ~Logger();
    /// @brief Obtain a stream to which you can push, incrementally creating the message
    std::ostream &get(HSTR_LOG_LEVEL log_level, HSTR_INFO_TYPE info_type);

private:
    /// @brief whether for a given log_level and info_type, the message should be logged
    bool shouldLog(HSTR_LOG_LEVEL log_level, HSTR_INFO_TYPE info_type);

    /// @brief get string representation for a given info type
    const char *getInfoTypeName(HSTR_INFO_TYPE info_type);

    /// @brief get string representation for a given logging level
    const char *getLogLevelName(HSTR_LOG_LEVEL log_level);
};

#endif

