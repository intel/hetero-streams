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

#include "hStreams_common.h"
#include "hStreams_internal.h"

namespace
{
const char *hStreamsResultNames[] = {
    "HSTR_RESULT_SUCCESS",
    "HSTR_RESULT_REMOTE_ERROR",
    "HSTR_RESULT_NOT_INITIALIZED",
    "HSTR_RESULT_NOT_FOUND",
    "HSTR_RESULT_ALREADY_FOUND",
    "HSTR_RESULT_OUT_OF_RANGE",
    "HSTR_RESULT_DOMAIN_OUT_OF_RANGE",
    "HSTR_RESULT_CPU_MASK_OUT_OF_RANGE",
    "HSTR_RESULT_OUT_OF_MEMORY",
    "HSTR_RESULT_INVALID_STREAM_TYPE",
    "HSTR_RESULT_OVERLAPPING_RESOURCES",
    "HSTR_RESULT_DEVICE_NOT_INITIALIZED",
    "HSTR_RESULT_BAD_NAME",
    "HSTR_RESULT_TOO_MANY_ARGS",
    "HSTR_RESULT_TIME_OUT_REACHED",
    "HSTR_RESULT_EVENT_CANCELED",
    "HSTR_RESULT_INCONSISTENT_ARGS",
    "HSTR_RESULT_BUFF_TOO_SMALL",
    "HSTR_RESULT_MEMORY_OPERAND_INCONSISTENT",
    "HSTR_RESULT_NULL_PTR",
    "HSTR_RESULT_INTERNAL_ERROR",
    "HSTR_RESULT_RESOURCE_EXHAUSTED",
    "HSTR_RESULT_NOT_IMPLEMENTED",
    "HSTR_RESULT_NOT_PERMITTED"
};
} // anonymous namespace

HSTR_STATIC_ASSERT(
    sizeof(hStreamsResultNames) == HSTR_RESULT_SIZE *sizeof(const char *),
    invalid_number_of_hStreamsResultNames_entries
);


HSTR_EXPORT_IN_VERSION(
    const char *,
    hStreams_ResultGetName,
    HSTREAMS_1.0)(HSTR_RESULT n)
{
    return hStreams_ResultGetName(n);
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    const char *,
    hStreams_ResultGetName)(HSTR_RESULT n)
{
    if (n >= 0 && n < HSTR_RESULT_SIZE) {
        return hStreamsResultNames[n];
    } else {
        return "[HSTR_RESULT value out of range]";
    }
}
