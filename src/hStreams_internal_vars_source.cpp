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
#include "hStreams_internal.h"

namespace globals
{

namespace initial_values
{
const HSTR_LOG_STR app_init_next_log_str_ID = 0;
const HSTR_LOG_DOM next_log_dom_id = 1;
const HSTR_MKL_INTERFACE mkl_interface = HSTR_MKL_LP64;
const char *interface_version = "[unknown]";
hStreams_Atomic_HSTR_STATE hStreamsState = HSTR_STATE_UNINITIALIZED;

const HSTR_OPTIONS options = {
    NULL,
    exit,
    0,
    HSTR_KMP_AFFINITY_BALANCED,
    HSTR_DEP_POLICY_BUFFERS,
    32, // limit on physical domains
    HSTR_OPENMP_PRE_SETUP,
    -1, // infinite timeout
    0,
    NULL,
    NULL,
    0,
    NULL
};
} // namespace initial_values

HSTR_LOG_STR app_init_next_log_str_ID = initial_values::app_init_next_log_str_ID;
std::vector<HSTR_LOG_DOM> app_init_log_doms_IDs;
std::string interface_version = initial_values::interface_version;
extern const std::array<const std::string, 5> supported_interface_versions = {
    HSTR_MAGIC_PRE1_0_0_VERSION_STRING,
    "1.0",
    "1.1",
    "1.2",
    "1.3"
};
hStreams_Atomic_HSTR_STATE hStreamsState = initial_values::hStreamsState;

HSTR_OPTIONS options = initial_values::options;
hStreams_RW_Lock options_lock;

std::map<HSTR_ISA_TYPE, std::vector<std::pair<std::string, int>>> libraries_to_load;
hStreams_RW_Lock libraries_to_load_lock;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4324 ) /* Disable warning for: 'lastError' : structure was padded due to __declspec(align()) */
#endif
HSTR_ALIGN(64) volatile int64_t next_log_dom_id = initial_values::next_log_dom_id;
#ifdef _WIN32
#pragma warning( pop )
#endif

std::string target_library_search_path;
std::string host_library_search_path;

std::vector<std::string> tokenized_target_library_search_path;
std::vector<std::string> tokenized_host_library_search_path;
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
HSTR_ALIGN(64) volatile int64_t huge_page_usage_threshold = -1;
#ifdef _WIN32
#pragma warning( pop )
#endif

const char *host_sink_ld_library_path_env_name = "HOST_SINK_LD_LIBRARY_PATH";
const char *sink_ld_library_path_env_name = "SINK_LD_LIBRARY_PATH";
const char *mic_ld_library_path_env_name = "MIC_LD_LIBRARY_PATH";

const uint32_t fixed_buffer_actions_cleanup_value = 100;
