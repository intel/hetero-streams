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

// /////////////////////////////////////////////////////////////////
// Purpose: Contains definition of global variables used across
//  all hStreams. Any file that would like to use them must declare
//  them as extern variables by including hStreams_internal_vars_source.h
// /////////////////////////////////////////////////////////////////

#include "hStreams_internal_vars_source.h"

namespace globals
{

namespace initial_values
{
const HSTR_LOG_STR app_init_next_log_str_ID = 0;
const HSTR_LOG_DOM next_log_dom_id = 1;
const HSTR_MKL_INTERFACE mkl_interface = HSTR_MKL_LP64;
} // namespace initial_values

HSTR_LOG_STR app_init_next_log_str_ID = initial_values::app_init_next_log_str_ID;
std::vector<HSTR_LOG_DOM> app_init_log_doms_IDs;

#ifdef WIN32
HMODULE hstreams_source_dll_handle;
#endif

} // namespace globals


//Main data structure
hStreamHostProcess hstr_proc;

hStreams_RW_Lock phys_domains_lock;
hStreams_PhysDomainCollection phys_domains;

hStreams_RW_Lock log_domains_lock;
hStreams_LogDomainCollection log_domains;

hStreams_RW_Lock log_streams_lock;
hStreams_LogStreamCollection log_streams;

hStreams_RW_Lock log_buffers_lock;
hStreams_LogBufferCollection log_buffers;


#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4324 ) /* Disable warning for: 'lastError' : structure was padded due to __declspec(align()) */
#endif
HSTR_ALIGN(64) volatile int64_t next_log_dom_id = globals::initial_values::next_log_dom_id;
HSTR_ALIGN(64) volatile int64_t huge_page_usage_threshold = -1;
#ifdef _WIN32
#pragma warning( pop )
#endif

const char *host_sink_ld_library_path_env_name = "HOST_SINK_LD_LIBRARY_PATH";
const char *mic_sink_ld_library_path_env_name = "SINK_LD_LIBRARY_PATH";
