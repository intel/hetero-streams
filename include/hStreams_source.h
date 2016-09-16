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

#ifndef __HSTR_SOURCE_H__
#define __HSTR_SOURCE_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "hStreams_version.h"
#include <wchar.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sched.h>
#endif
#include "hStreams_types.h"
#include "hStreams_common.h"


#ifdef _WIN32
#   include <stdint.h>
#else
#   include <stdbool.h>
#   include <sched.h>
#endif
#endif //DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////
// Doxygen settings
/// @file include/hStreams_source.h
///
/// @defgroup hStreams_Source hStreams Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_General hStreams Source - General
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_Domains hStreams Source - Domains
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_StreamMgmt hStreams Source - Stream management
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_StreamUsage hStreams Source - Stream usage
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_Sync hStreams Source - Sync
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_MemMgmt hStreams Source - Memory management
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Source_Errors hStreams Source - Error handling
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Configuration hStreams Source - Configuration
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_Utils hStreams Utilities
/// @ingroup hStreams_Source
// TODO: Description for this group
//
/// @defgroup hStreams_CPUMASK CPU_MASK manipulating
/// @ingroup hStreams_Utils
/// Functions used for manipulating HSTR_CPU_MASK.
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
///
// hStreams_Init
/// @ingroup hStreams_Source_General
/// @brief Initialize hStreams-related state.
///
///  Must be called before all other hStreams functions,
///   else HSTR_RESULT_NOT_INITIALIZED will result.
///
///  Note that hStreams_Init() will load the sink-side libraries during initialization.
///  To load the sink-side libraries, hStreams_Init() first, attempts to find a sibling
///  file as the source-side executable with suffix "_mic.so".  For example if the
///  source-side executable is called 'test_app', then, hStreams_Init() attempts to
///  find a file named 'test_app_mic.so' somewhere in the list of directories defined
///  in the SINK_LD_LIBRARY_PATH environment variable.  (On windows, the '.exe'
///  extension is removed from the source-side executable file name before the sibling
///  file is sought).  Next, if the HSTR_OPTIONS struct contains a number of libraries,
///  they will be loaded, again searching the SINK_LD_LIBRARY_PATH for each of the
///  libraries specified there.
///
///
/// @return If successful, \c hStreams_InitInVersion() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_DEVICE_NOT_INITIALIZED if the library cannot be initialized properly,
///     e.g. because no MICs are available
///
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the \c interface_version argument does not correspond
///     to a valid interface version supported by the library
///
/// @thread_safety Concurrent calls to \c hStreams_InitInVersion() are serialized internally by the
///     implementation.  The first thread to acquire exclusivity will attempt to initialize the
///     library. If the initialisation is successfull, the first thread will return \c
///     HSTR_RESULT_SUCCESS as well as all of the other threads that were serialized. If the
///     initialisation of the library by the first thread fails, the first thread shall return an
///     error code and the second thread to acquire exclusivity will again try to initialize the
///     library.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_InitInVersion(const char *interface_version);

static inline HSTR_RESULT
hStreams_Init()
{
    return hStreams_InitInVersion(HSTR_VERSION_STRING);
}

/////////////////////////////////////////////////////////
///
// hStreams_IsInitialized
/// @ingroup hStreams_Source_General
/// @brief Check if hStreams has been initialised properly.
///
/// @return If successful, \c hStreams_IsInitialized() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
///
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @thread_safety Thread safe.
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_IsInitialized();

/////////////////////////////////////////////////////////
///
// hStreams_Fini
/// @ingroup hStreams_Source_General
/// @brief Finalize hStreams-related state.
///
/// Destroys hStreams internal structures and clears the state of the library.
/// All logical domains, streams and buffers are destroyed as a result of this call.
///
/// @return If successful, \c hStreams_Fini() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Fini();

////////////////////////////////////////////////////////
///
// hStreams_GetNumPhysDomains
/// @ingroup hStreams_Source_Domains
/// @brief Returns number of discovered and active physical domains
///
///  This API covers only static information about physical domains.
///  hStreams_GetAvailable reveals unused threads for a given domain.
///
/// @param  out_pNumPhysDomains
///         [out] Number of sink physical domains, e.g. MIC accelerator cards. This number
///         cannot be higher \c phys_domains_limit set in \c HSTR_OPTIONS.
///         See \c hStreams_Init() for more details.
///
/// @param  out_pNumActivePhysDomains
///         [out] Number of active domains, at initialization time
///
/// @param  out_pHomogeneous
///         [out] True if all physical domains besides host have the same resources and
///         capabilities, else false
///
/// @return If successful, \c hStreams_GetNumPhysDomains() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pNumPhysDomains is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pNumActivePhysDomains is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pHomogeneous is \c NULL.
///
/// @thread_safety Thread safe.
///
////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetNumPhysDomains(
    uint32_t          *out_pNumPhysDomains,
    uint32_t          *out_pNumActivePhysDomains,
    bool              *out_pHomogeneous);

////////////////////////////////////////////////////////
///
// hStreams_GetPhysDomainDetails
/// @ingroup hStreams_Source_Domains
/// @brief Returns information about specified physical domain.
///
///  This API covers only static information about a single physical domain.
///  hStreams_GetAvailable reveals unused threads for a given physical domain.
///
/// @param  in_PhysDomain
///         [in] Physical domain ID: \c HSTR_SRC_PHYS_DOMAIN for source, remote domains start at 0
///
/// @param  out_pNumThreads
///         [out] Number of threads (not cores) on each domain; 0 means not initialized
///
/// @param  out_pISA
///         [out] ISA type
///         Anticipated improvement: provide a routine to convert this to text
///
/// @param  out_pCoreMaxMHz
///         [out] Maximum core frequence, in MHz
///
/// @param  out_MaxCPUmask
///         [out] hStreams cpu mask covering all available threads on each domain
///
/// @param  out_AvoidCPUmask
///         [out] hStreams cpu mask covering all threads that are normally reserved for OS use.
///         The nominally-available recommended CPU mask is the XOR of MaxCPUmax and AvoidCPUmask.
///
/// @param  out_pSupportedMemTypes
///         [out] bit array mask of memory types supported for this physical domain,
///         That is, bit 0 is HSTR_MEM_TYPE_NORMAL, bit 1 is HSTR_MEM_TYPE_HBW, etc.
///
/// @param  out_pPhysicalBytesPerMemType
///         [out] array of physical sizes, in bytes, of each memory type, in this physical domain
///         The size of the array passed in must be HSTR_MEM_TYPE_SIZE
///         The array elements correspond to the enumeration in HSTR_MEM_TYPE
///         The value is 0 for memory types that are defined but unsupported on that domain
///
/// @return If successful, \c hStreams_GetPhysDomainDetails() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_PhysDomain is out of range or
///     provided physical domain is inactive.
/// @arg \c HSTR_RESULT_NULL_PTR, if an out_pNumThreads is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if an out_pISA is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if an out_pCoreMaxMHz is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if an out_pSupportedMemTypes is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR, if an out_pPhysicalBytesPerMemType is \c NULL.
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetPhysDomainDetails(
    HSTR_PHYS_DOM       in_PhysDomain,
    uint32_t           *out_pNumThreads,
    HSTR_ISA_TYPE      *out_pISA,
    uint32_t           *out_pCoreMaxMHz,
    HSTR_CPU_MASK       out_MaxCPUmask,
    HSTR_CPU_MASK       out_AvoidCPUmask,
    uint64_t           *out_pSupportedMemTypes,
    uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE]);

////////////////////////////////////////////////////////
///
// hStreams_GetAvailable
/// @ingroup hStreams_Source_Domains
/// @brief Returns unused yet cpu threads
///
///  This API covers only dynamic information, to complement GetPhysDomainDetails
///  hStreams_GetAvailable reveals unused threads for a given physical domain.
///
/// @param  in_PhysDomainID
///         [in] Index of domain
///         Domain numbering starts with 0 for domains, HSTR_SRC_PHYS_DOMAIN
///          for the host.
///
/// @param  out_AvailableCPUmask
///         [out] hStreams cpu mask covering all threads on specific domain
///         that are currently not associated with any stream
///
/// @return If successful, \c hStreams_GetAvailable() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_PhysDomainID is out of range or
///     provided physical domain is inactive.
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetAvailable(
    HSTR_PHYS_DOM       in_PhysDomainID,
    HSTR_CPU_MASK       out_AvailableCPUmask);

/////////////////////////////////////////////////////////
///
// hStreams_AddLogDomain
/// @ingroup hStreams_Source_Domains
/// @brief Create a new logical domain in a physical domain.
///
/// The CPU resources specified by a logical domain's CPU mask are not allowed to
/// partially overlap within the resources of another logical domain's in the same
/// physical domain. They must be either disjoint or fully overlap each other. Please
/// note that logical domains with exactly overlapping CPU resources are considered
/// to be distinct entities, i.e. they do not alias the same entity as is the case with
/// exactly overlapping logical streams.
///
/// Logical domain IDs are generated by hStreams.
///
/// @param  in_PhysDomainID
///         [in] ID of the physical domain to add logical domain to
///
/// @param  in_CPUmask
///         [in] HW threads mask that the logical domain is bound to
///
/// @param  out_pLogDomainID
///         [out] Generated logical domain ID
///
/// @param  out_pOverlap
///         [out] Resulting overlap status
///
/// @return If successful, \c hStreams_AddLogDomain() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if there is no valid physical domain with ID \c in_PhysDomainID
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pLogDomainID is \c NULL
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pOverlap is \c NULL
///     correspond to a valid logical domain
/// @arg \c HSTR_RESULT_OVERLAPPING_RESOURCES, if \c in_CPUmask partially
///     overlaps any logical domain previously added in physical domain \c
///     in_PhysDomainID
/// @arg \c HSTR_RESULT_CPU_MASK_OUT_OF_RANGE if \c in_CPUmask is empty
/// @arg \c HSTR_RESULT_CPU_MASK_OUT_OF_RANGE if \c in_CPUmask is outside the range
///     of the \c in_PhysDomainID physical domain
/// @arg \c HSTR_RESULT_OUT_OF_MEMORY if any incremental buffer cannot be
///     instantiated because of memory exhaustion or unavailability of requested
///     memory kind, subject to the buffer's memory allocation policy.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_AddLogDomain(
    HSTR_PHYS_DOM      in_PhysDomainID,
    HSTR_CPU_MASK      in_CPUmask,
    HSTR_LOG_DOM      *out_pLogDomainID,
    HSTR_OVERLAP_TYPE *out_pOverlap);

/////////////////////////////////////////////////////////
///
// hStreams_RmLogDomains
/// @ingroup hStreams_Source_Domains
/// @brief Remove logical domains.
///
/// This function will remove listed logical domains from their physical
/// domains, make their logical domain IDs invalid and remove any associated
/// logical streams.
///
/// @note A call to \c hStreams_RmLogDomains() will block until the streams from all
///     logical domains listed in the \c in_pLogDomainIDs array have processed all
///     the enqueued actions.
///
/// @param  in_NumLogDomains
///         [in] Number of entries in the \c in_pLogDomainIDs array
///
/// @param  in_pLogDomainIDs
///         [in] Array of IDs of logical domains to be removed
///
/// @return If successful, \c hStreams_RmLogDomains() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pLogDomainIDs is NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if at least one of the logical domain IDs doesn't
///     correspond to a valid logical domain
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_NumLogDomains equals 0
///
/// @thread_safety  Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_RmLogDomains(
    uint32_t       in_NumLogDomains,
    HSTR_LOG_DOM  *in_pLogDomainIDs);

/////////////////////////////////////////////////////////
///
// hStreams_GetNumLogDomains
/// @ingroup hStreams_Source_Domains
/// @brief Return number logical domains associated with a physical domain.
///
/// @param  in_PhysDomainID
///         [in]  Physical domain ID: \c HSTR_SRC_PHYS_DOMAIN for source,
///             remote domains start at 0
///
/// @param  out_pNumLogDomains
///         [out] Number of logical domains associated with \c in_PhysDomainID
///
/// @return If successful, \c hStreams_GetNumLogDomains() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_PhysDomainID is out of range or
///     provided physical domain is inactive.
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pNumLogDomains is \c NULL
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetNumLogDomains(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t       *out_pNumLogDomains);

/////////////////////////////////////////////////////////
///
// hStreams_GetLogDomainIDList
/// @ingroup hStreams_Source_Domains
///
/// @brief Returns list of logical domains attached to provided physical domain.
///
/// @note Before calling \c hStreams_GetLogDomainIDList you should call \c
///         hStreams_GetNumLogDomains() function to get number of logical
///         domains attached to provided physical domain.
///
/// @param  in_PhysDomainID
///         [in] ID of physical domain for which we are querying
///
/// @param  in_NumLogDomains
///         [in] Number of allocated by user elements inside \c out_pLogDomainIDs array
///
/// @param  out_pLogDomainIDs
///         [out] Array with \c in_NumLogDomains logical domain IDs
///
/// @return If successful, \c hStreams_GetLogDomainIDList() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_PhysDomainID is out of range or
///     provided physical domain is inactive.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pLogDomainIDs is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if fewer than \c in_NumLogDomains mappings found
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if more than \c in_NumLogDomains
///         mappings are found.
///
/// @thread_safety Thread safe. Please note that subsequent calls to \c hStreams_GetNumLogDomains()
///     and \c hStreams_GetLogDomainIDList() do not form an atomic operation and thus logical
///     domains might have been added or removed between the calls to these APIs.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetLogDomainIDList(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumLogDomains,
    HSTR_LOG_DOM   *out_pLogDomainIDs);


/////////////////////////////////////////////////////////
///
// hStreams_GetLogDomainDetails
/// @ingroup hStreams_Source_Domains
/// @brief Returns associated cpu mask and physical domain to provided logical domain
///
/// @param  in_LogDomainID
///         [in] ID of logical domain to look up
///         Logical domains are associated with exactly one physical domain.
///
/// @param  out_pPhysDomainID
///         [out] physical domain ID associated with provided logical domain
///
/// @param  out_CPUmask
///         [out] cpu mask associated with provided logical domain
///
/// @return If successful, \c hStreams_GetLogDomainDetails() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogDomainID is out of range
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pPhysDomainID is NULL
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetLogDomainDetails(
    HSTR_LOG_DOM   in_LogDomainID,
    HSTR_PHYS_DOM *out_pPhysDomainID,
    HSTR_CPU_MASK  out_CPUmask);

/////////////////////////////////////////////////////////
///
// hStreams_StreamCreate
/// @ingroup hStreams_Source_StreamMgmt
/// @brief Register a logical stream and specify its domain and CPU mask.
///
/// @note The logical stream IDs are assigned by the user and do not need to form a contiguous range.
///
/// @note Multiple logical streams within the same logical domain are permitted to fully
///     or partially overlap. Partially overlapping streams do not have any association
///     between them - they simply share the same CPU resources. However, logical streams
///     which fully overlap map to the same physical stream.
///
/// @param  in_LogStreamID
///         [in] The ID of the logical stream to be created.
///
/// @param  in_LogDomainID
///         [in] The ID of the logical domain to create the stream in.
///
/// @param  in_CPUmask
///         [in] The mask describing the HW threads that are used by this logical
///         stream.
///
/// @return If successful, \c hStreams_StreamCreate() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_ALREADY_FOUND if there already exists a logical stream with ID
///     equal to \c in_LogStreamID
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_LogDomainID doesn't correspond
///     to a valid logical domain
/// @arg \c HSTR_RESULT_CPU_MASK_OUT_OF_RANGE if \c in_CPUmask is empty
/// @arg \c HSTR_RESULT_CPU_MASK_OUT_OF_RANGE if \c in_CPUmask doesn't fall
///     entirely within the CPU mask for logical domain \c in_LogDomainID
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_StreamCreate(
    HSTR_LOG_STR        in_LogStreamID,
    HSTR_LOG_DOM        in_LogDomainID,
    const HSTR_CPU_MASK in_CPUmask);

/////////////////////////////////////////////////////////
///
// hStreams_StreamDestroy
/// @ingroup hStreams_Source_StreamMgmt
/// @brief Destroy a logical stream
///
/// @note A call to \c hStreams_StreamDestroy() will block until all the actions
///     thus far enqueued in the stream have completed.
///
/// @note If multiple logical streams map to the same physical stream (i.e. they
///     belong to the same logical domain and their CPU masks fully overlap), only
///     the destruction of the last logical stream from that set will result in
///     releasing the underlying resources.
///
/// @param  in_LogStreamID
///         [in] The ID of the logical stream to be destroyed
///
/// @return If successful, \c hStreams_StreamDestroy() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID doesn't correspond to a valid
///     logical stream
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_StreamDestroy(
    HSTR_LOG_STR in_LogStreamID);

/////////////////////////////////////////////////////////
///
// hStreams_GetNumLogStreams
/// @ingroup hStreams_Source_StreamMgmt
/// @brief Return number of logical streams associated with a logical domain.
///
/// @param  in_logDomainID
///         [in] ID of logical domain to be queried
///
/// @param  out_pNumLogStreams
///         [out] Number of logical streams associated with \c in_LogDomainID
///
/// @return If successful, \c hStreams_GetNumLogStreams() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pNumLogStreams is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if in_LogDomainID is not valid
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetNumLogStreams(
    HSTR_LOG_DOM     in_LogDomainID,
    uint32_t        *out_pNumLogStreams);

/////////////////////////////////////////////////////////
///
// hStreams_GetLogStreamIDList
/// @ingroup hStreams_Source_StreamMgmt
/// @brief Returns list of logical streams attached to provided logical domain.
///
/// @note Before calling \c hStreams_GetLogStreamIDList you should call \c
///         hStreams_GetNumLogStreams() function to get number of logical
///         streams attached to provided logical domain.
///
/// @param  in_LogDomainID
///         [in] ID of logical domain for which we are querying
///
/// @param  in_NumLogStreams
///         [in] Number of allocated by user elements inside \c out_pLogStreamIDs array
///
/// @param  out_pLogStreamIDs
///         [out] Array with \c in_NumLogStreams logical stream IDs
///
/// @return If successful, \c hStreams_GetLogStreamIDList() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if \c in_LogDomainID is out of range
/// @arg \c HSTR_RESULT_NULL_PTR, if \c out_pLogStreamIDs is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if fewer than \c in_NumLogStreams mappings found
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if more than \c in_NumLogStreams
///         mappings are found.
///
/// @thread_safety Thread safe. Please note that subsequent calls to \c hStreams_GetNumLogStreams()
///     and \c hStreams_GetLogStreamIDList() do not form an atomic operation and thus logical
///     streams might have been added or removed between the calls to these APIs.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetLogStreamIDList(
    HSTR_LOG_DOM  in_LogDomainID,
    uint32_t      in_NumLogStreams,
    HSTR_LOG_STR *out_pLogStreamIDs);

/////////////////////////////////////////////////////////
///
// hStreams_GetLogStreamDetails
/// @ingroup hStreams_Source_StreamMgmt
/// @brief Returns cpu mask assigned to provided logical stream
///
/// @param  in_LogSstreamID
///         [in] ID of logical stream to look up
///
/// @param  in_LogDomainID
///         [in] ID of logical domain to look up
///
/// @param  out_CPUmask
///         [out] cpu mask associated with provided logical stream
///
/// @return If successful, \c hStreams_GetLogStreamDetails() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID is out of range
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetLogStreamDetails(
    HSTR_LOG_STR      in_LogStreamID,
    HSTR_LOG_DOM      in_LogDomainID,
    HSTR_CPU_MASK     out_CPUmask);

/////////////////////////////////////////////////////////
///
// hStreams_GetOversubscriptionLevel
/// @ingroup hStreams_Source_StreamUsage
/// @brief Query the number of streams overlapping for each HW thread.
///
/// Given a physical domain ID, fill an array with oversubscription level for
/// each HW thread. \c in_NumThreads must be equal to the number of HW threads
/// existing on queried physical domain. The number of HW threads can be
/// obtained from \c hStreams_GetPhysDomainDetails by checking the output
/// parameter \c out_pNumThreads.
///
/// @param  in_PhysDomainID
///         [in] ID of the physical domain to query.  Enumerated physical
///         domain IDs can be \c HSTR_SRC_PHYS_DOMAIN or 0 and higher for
///         non-source physical domains.
///
/// @param  in_NumThreads
///         [in] The number of entries in the \c out_pOversubscriptionArray array
///
/// @param  out_pOversubscriptionArray
///         [out] The array of elements describing a number of streams using a
///         HW thread corresponding to this element index.
///
/// @return If successful, \c hStreams_GetOversubscriptionLevel() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pOversubscriptionArray is \c NULL.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_NumThreads is not equal to the number
///         of HW threads existing on the queried physical domain.
///
/// @thread_safety Thread safe for operations of different output arrays.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetOversubscriptionLevel(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumThreads,
    uint32_t       *out_pOversubscriptionArray);

/////////////////////////////////////////////////////////
///
// hStreams_EnqueueCompute
/// @ingroup hStreams_Source_StreamUsage
/// @brief Enqueue an execution of a user-defined function in a stream
///
/// Places an execution of a user-defined function in the stream's internal queue.
/// The function to be called shall be compiled and loaded to the sink process so
/// that hStreams can locate the appropriate symbol and invoke it on the stream's
/// sink endpoint.
///
/// @param  in_LogStreamID
///         [in] ID of logical stream associated to enqueue the action in
///
/// @param  in_pFuncName
///         [in] Null-terminated string with name of the function to be executed
///
/// @param  in_NumScalarArgs
///         [in] Number of arguments to be copied by value for remote invocation
///
/// @param  in_NumHeapArgs
///         [in] Number of arguments which are buffer addreses to be translated
///         to sink-side instantiations' addresses
///
/// @param  in_pArgs
///         [in] Array of in_NumScalarArgs+in_NumHeapArgs arguments as 64-bit unsigned
///         integers with scalar args first and buffer args second
///
/// @param  out_pEvent
///         [out] pointer to event which will be signaled once the action
///         completes
///
/// @param  out_pReturnValue
///         [out] pointer to host-side memory the remote invocation can
///         asynchronously write to
///
/// @param  in_ReturnValueSize
///         [in] the size of the asynchronous return value memory
///
/// @return If successful, \c hStreams_EnqueueCompute() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID is not a valid logical stream ID
/// @arg \c HSTR_RESULT_NOT_FOUND if at least one of the buffer arguments is not
///     in a buffer that had been instantiated for \c in_LogStreamID's logical domain
/// @arg \c HSTR_RESULT_BAD_NAME if \c in_pFunctionName is \c NULL
/// @arg \c HSTR_RESULT_BAD_NAME if no symbol named \c in_pFunctionName is found
///     on the streams's sink endpoint
/// @arg \c HSTR_RESULT_BAD_NAME if \c in_pFunctionName is longer than \c
///     HSTR_MAX_FUNC_NAME_SIZE
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_ReturnValueSize exceeds \c
///     HSTR_RETURN_SIZE_LIMIT
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if <tt>in_ReturnValueSize != 0</tt>
///     and \c in_pReturnValue is \c NULL or <tt>in_ReturnValueSize == 0</tt> and \c
///     in_pReturnValue is not \c NULL
/// @arg \c HSTR_RESULT_TOO_MANY_ARGS if <tt>in_NumScalarArgs +
///     in_NumHeapArgs > HSTR_ARGS_SUPPORTED</tt>
/// @arg \c HSTR_RESULT_NULL_PTR <tt>in_numScalarArgs + in_numHeapArgs > 0</tt> but
///     \c in_pArgs is \c NULL
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_EnqueueCompute() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_EnqueueCompute(
    HSTR_LOG_STR   in_LogStreamID,
    const char    *in_pFunctionName,
    uint32_t       in_numScalarArgs,
    uint32_t       in_numHeapArgs,
    uint64_t      *in_pArgs,
    HSTR_EVENT    *out_pEvent,
    void          *out_ReturnValue,
    uint16_t       in_ReturnValueSize);


/////////////////////////////////////////////////////////
///
// hStreams_EnqueueData1D
/// @ingroup hStreams_Source_StreamUsage
/// @brief Enqueue 1-dimensional data transfers in a logical stream.
///
/// @note Data transfers are permitted to execute out-of-order subject to dependence
///     policy and the dependences previously inserted into the stream.
/// @sa \c HSTR_OPTIONS.dep_policy
///
/// @note This API allows for memory transfers between the \e source and the \e
///     sink logical domain, not between arbitrary sink logical domain.
/// @sa \c hStreams_EnqueueDataXDomain1D() for an API allowing transfers between
///     arbitrary logical domains
///
/// @param  in_LogStreamID
///         [in] The ID of the logical stream to insert the data transfer action in.
///
/// @param  in_pWriteAddr
///         [in] Source proxy pointer to the memory location to write to
///
/// @param  in_pReadAddr
///         [in] Source proxy pointer to the memory location to read from
///
/// @param  in_size
///         [in] The size, in bytes, of contiguous memory that should be copied
///
/// @param  in_XferDirection
///         [in] The direction in which the memory transfer should occur
///
/// @param  out_pEvent
///         [out] optional, the pointer for the completion event
///
/// @return If successful, \c hStreams_EnqueueData1D() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pWriteAddr is \c NULL
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pReadAddr is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if there is no logical stream with ID equal to
///     \c in_LogStreamID
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pWriteAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pReadAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the data locations involved in the transfer
///     fall outside of any of the buffers' boundaries.
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_EnqueueData1D() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_EnqueueData1D(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent);

/////////////////////////////////////////////////////////
///
// hStreams_EnqueueDataXDomain1D
/// @ingroup hStreams_Source_StreamUsage
/// @brief Enqueue 1-dimensional data between an arbitrary domain and one of the
///        endpoint domains of this stream
///
/// @note Data transfers are permitted to execute out-of-order subject to dependence
///     policy and the dependences previously inserted into the stream.
/// @sa \c HSTR_OPTIONS.dep_policy
///
/// @param  in_LogStreamID
///         [in] The ID of the logical stream to insert the data transfer action in.
///
/// @param  in_pWriteAddr
///         [in] Source proxy pointer to the memory location to write to
///
/// @param  in_pReadAddr
///         [in] Source proxy pointer to the memory location to read from
///
/// @param  in_size
///         [in] The size, in bytes, of contiguous memory that should be copied
///
/// @param  in_destLogDomain
///         [in] The logical domain to which to transfer the data
///
/// @param  in_srcLogDomain
///         [in] The logical domain from which to transfer the data
///
/// @param  out_pEvent
///         [out] optional, the pointer for the completion event
///
/// @return If successful, \c hStreams_EnqueueDataXDomain1D() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if the library had not been initialized properly
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pWriteAddr is \c NULL
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pReadAddr is \c NULL
/// @arg \c HSTR_RESULT_NOT_FOUND if there is no logical stream with ID equal to
///     \c in_LogStreamID
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pWriteAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_pReadAddr does not fall in a buffer with
///     a valid instantiation for the logical domain \c in_LogStreamID is located in.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the data locations involved in the transfer
///     fall outside of any of the buffers' boundaries.
/// @arg \c HSTR_RESULT_OVERLAPPING_RESOURCES if transfer is enqueued within a
///     buffer that has the aliased buffer property set, source and destination
///     logical domains belong to one physical domain and the source and
///     destination regions partial overlap.
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_EnqueueDataXDomain1D() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_EnqueueDataXDomain1D(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_LOG_DOM        in_destLogDomain,
    HSTR_LOG_DOM        in_srcLogDomain,
    HSTR_EVENT         *out_pEvent);

/////////////////////////////////////////////////////////
///
// hStreams_StreamSynchronize
/// @ingroup hStreams_Source_Sync
/// @brief Block until all the operation enqueued in a stream have completed
///
/// @param  in_LogStreamID
///         [in] ID of the logical stream
///
/// @return If successful, \c hStreams_StreamSynchronize() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_LogStreamID is not a valid logical stream ID
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if timeout was reached while waiting on the
///     completion of stream's actions.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if one of the events in the stream was canceled
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
///
/// @see HSTR_OPTIONS.time_out_ms_val
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_StreamSynchronize(
    HSTR_LOG_STR in_LogStreamID);

/////////////////////////////////////////////////////////
///
// hStreams_ThreadSynchronize
/// @ingroup hStreams_Source_Sync
/// @brief Block until all the operation enqueued in all the streams have completed
///
/// @return If successful, \c hStreams_ThreadSynchronize() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if timeout was reached while waiting on the
///     completion of actions.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if one of the events was canceled
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
///
/// @see HSTR_OPTIONS.time_out_ms_val
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_ThreadSynchronize();

/////////////////////////////////////////////////////////
///
// hStreams_EventWait
/// @ingroup hStreams_Source_Sync
/// @brief Wait on a set of events.
///
/// @param  in_NumEvents
///         [in] number of event pointers in the array
///
/// @param  in_pEvents
///         [in] the array of pointers of events to be waited on
///
/// @param  in_WaitForAll
///         [in] If true, report success only if all events signaled
///         If false, report success if at least one event signaled
///
/// @param  in_TimeOutMilliSeconds
///         [in] Timeout.  0 polls and returns immediately, or HSTR_TIME_INFINITE (-1)
///
/// @param  out_pNumSignaled
///         [out] Number of events that signaled as completed
///
/// @param  out_pSignaledIndices
///         [out] Packed list of indices into in_pEvents that signaled
///
/// @return If successful, \c hSteramsEventWait() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns on of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NULL_PTR if in_pEvents is NULL
/// @arg \c HSTR_RESULT_REMOTE_ERROR if there was a remote error, e.g. the
///         remote process died
/// @arg \c HSTR_RESULT_TIME_OUT_REACHED if the time out was reached or the timeout is zero
///         and the event has not been signalled.
/// @arg \c HSTR_RESULT_EVENT_CANCELED if the event was cancelled or the process died
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if in_NumEvents == 0
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if in_NumEvents != 1 && !in_WaitForAll and
///         either of out_pNumSignaled or out_pSignaledIndices are NULL
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_EventWait(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    bool               in_WaitForAll,
    int32_t            in_TimeOutMilliSeconds,
    uint32_t          *out_pNumSignaled,
    uint32_t          *out_pSignaledIndices);


/////////////////////////////////////////////////////////
///
// hStreams_EventStreamWait
/// @ingroup hStreams_Source_Sync
/// @brief Aggregate multiple dependences into one event handle and optionally insert
///     that event handle into a logical stream
///
/// The resulting aggregation of multiple dependences is composed of depenences on:
/// \arg last actions actions enqueued in the stream which involve buffers specified in the
///     \c in_pAddresses array
/// \arg explicit completion events specified in the in_pEvents array
/// The output event handle, \c out_pEvent will represent that aggregation of dependences.
///
/// The special pair of values <tt>(-1, NULL)</tt> for <tt>(in_NumAddresses,
/// in_pAddresses)</tt> is used to signify that the dependence to be created should not
/// be inserted into the stream, only an event handle representing that dependence should
/// be returned.
///
/// The special pair of values <tt>(0, NULL)</tt> for <tt>(in_NumAddresses,
/// in_pAddresses)</tt> is used to signify that the dependence to be created should not
/// involve any actions related to buffer.
///
/// On the other hand, the special pair of values <tt>(0, NULL)</tt> for <tt>(in_NumEvents,
/// in_pEvents)</tt> is used to signify that the dependence to be inserted should include
/// all the actions previously enqueued in the specified stream.
///
/// @param  in_LogStreamID
///         [in] ID of the logical stream from which take the dependencies and into which
///         to optionally insert the dependence.
///
/// @param  in_NumEvents
///         [in] Number of entries in the \c in_pEvents array.
///
/// @param  in_pEvents
///         [in] An array of event handles that should be included in the aggregated dependence.
///
/// @param  in_NumAddresses
///         [in] Number of entries in the in_pAddresses array.
///
/// @param  in_pAddresses
///         [in] Array of source-side proxy addresses to be mapped to buffers, on which
///          the dependencies will be computed for aggregation.
///
/// @param  out_pEvent
///         [out] the aggreagated completion event. If no handle is needed, set this to NULL
///
/// @return If successful, \c hStreams_EventStreamWait() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized properly.
/// @arg \c HSTR_RESULT_NOT_FOUND if a logical stream with ID \c in_LogStreamID doesn't
///     exist
/// @arg \c HSTR_RESULT_NOT_FOUND if at least one entry in the \c in_pAddresses array
///     does not correspond to a buffer with an instantiation in the logical domain in
///     which the \c in_LogStreamID stream is located is located.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_NumAddresses is 0 or -1 and \c
///     in_pAddresses is not NULL and vice versa.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_NumEvents is 0 and \c
///     in_pEvents is not NULL and vice versa.
///
/// @thread_safety All actions enqueued through concurrent calls to \c
///     hStreams_EventStreamWait() and any other function that enqueues actions into
///     the same stream are guaranteed to be correctly inserted into the
///     stream's queue, although in an unspecified order.  Therefore,
///     concurrent calls to these functions which operate on the
///     same data will produce undefined results.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_EventStreamWait(
    HSTR_LOG_STR   in_LogStreamID,
    uint32_t       in_NumEvents,
    HSTR_EVENT    *in_pEvents,
    int32_t        in_NumAddresses,
    void         **in_pAddresses,
    HSTR_EVENT    *out_pEvent);

/////////////////////////////////////////////////////////
///
// hStreams_Alloc1D
/// @ingroup hStreams_AppApi_Core
/// @brief Allocate 1-dimensional buffer on each currently existing logical domains.
///
/// Construct an hStreams buffer out of user-provided memory. This function creates
/// an instantiation of the buffer in all of the currently present logical domains.
/// Note that the contents of those instatiations are considered to be undefined, i.e.
/// the contents of the buffer are not implicitly synchronized across all the
/// instantations.
///
/// A buffer constructed by a call to \c hStreams_Alloc1D() may be later supplied
/// as an operand to memory transfer or a compute action. In order to do that, user
/// should supply a valid address falling anywhere \em inside the buffer.
///
/// @param  in_BufAddr
///         [in] pointer to the beginning of the memory in the source logical domain
///
/// @param  in_NumBytes
///         [in] size of the memory to create the buffer for, in bytes
///
///
/// @return If successful, \c hStreams_Alloc1D() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_BaseAddress is \c NULL
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumBytes == 0</tt>
/// @arg \c HSTR_RESULT_OUT_OF_MEMORY if there's not enough memory on one or
///     more domains and therefore the buffer cannot be instantiatied
///
/// @thread_safety Thread safe.
///
////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Alloc1D(
    void               *in_BaseAddress,
    uint64_t            in_size);

////////////////////////////////////////////////////////
///
// hStreams_Alloc1DEx
/// @ingroup hStreams_Source_MemMgmt
/// @brief Allocate 1-dimensional buffer with additional properties.
///
/// This API is an extended version of \c hStreams_Alloc1D(), allowing for
/// controlling the properties of the buffer as well as instantiation of the
/// buffer on only selected logical domains.
///
/// As with \c hStreams_Alloc1D(), the buffer must be created from existing
/// source memory, so as to have a source proxy address by which to identify it.
/// The resulting buffer is always instantiated for the special logical domain
/// \c HSTR_SRC_LOG_DOMAIN.
/// Note that the contents of those instatiations are considered to be undefined, i.e.
/// the contents of the buffer are not implicitly synchronized across all the
/// instantations.
///
/// If \c NULL is supplied to the buffer properties argument (\c
/// in_pBufferProps), default properties will be used. See \c HSTR_BUFFER_PROPS
/// documentation for details on what the default properties are.
///
/// If \c 0 is supplied as the number of logical domains (in_NumLogDomains) for
/// which to instantiate the buffer, \c in_pLogDomainIDs must be \c NULL - the buffer
/// will not be instantiated for any logical domain except \c HSTR_SRC_LOG_DOMAIN.
///
/// If <tt>-1</tt> is supplied as the number of logical domains (in_NumLogDomains) for
/// which to instantiate the buffer, \c in_pLogDomainIDs must be \c NULL - the buffer
/// will be instantiated for all logical domains already present.
///
/// @param  in_BaseAddress
///         [in] pointer to the beginning of the memory in the source logical domain
/// @param  in_Size
///         [in] size of the memory to create the buffer for, in bytes
/// @param  in_pBufferProps
///         [in] Buffer properties, optional.
/// @param  in_NumLogDomains
///         [in] Number of logical domains to instantiate the buffer in.
/// @param  in_pLogDomainIDs
///         [in] Array of logical domains IDs for which to instantiate the buffer.
///         The array must not contain HSTR_SRC_LOG_DOMAIN.
///
/// @return If successful, \c hStreams_Alloc1DEx() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_Size == 0</tt>
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumLogDomains < -1</tt>
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if \c in_pBufferProps contains an invalid value.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_BaseAddress is \c NULL.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if
///     <tt>in_NumLogDomains == -1 && in_pLogDomainIDs != NULL</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if
///     <tt>in_NumLogDomains == 0 && in_pLogDomainIDs != NULL</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_pLogDomainIDs array contains
///     duplicate entries.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if at least one entry in the
///     \c in_pLogDomainIDs array is not a valid logical domain ID.
/// @arg \c HSTR_RESULT_ALREADY_FOUND if a buffer had been previously created
///     with the same memory.
/// @arg \c HSTR_RESULT_OVERLAPPING_RESOURCES if the memory range partially overlaps
///     one or more buffers which had been created previously.
/// @arg \c HSTR_RESULT_OUT_OF_MEMORY if one or more of the buffer's instatiations
///     cannot be created due to the sink domain's memory exhaustion or unavailability
///     of the requested memory kind, subject to memory allocation policy specified
///     in the buffer's properties.
/// @arg \c HSTR_RESULT_NOT_IMPLEMENTED if requested memory type is other than
///     \c HSTR_MEM_TYPE_NORMAL.
/// @arg \c HSTR_RESULT_NOT_IMPLEMENTED if \c HSTR_BUF_PROP_AFFINITIZED flag is set
///     in the buffer's properties.
///
/// @thread_safety Thread safe.
///
////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Alloc1DEx(
    void                 *in_BaseAddress,
    uint64_t              in_Size,
    HSTR_BUFFER_PROPS    *in_pBufferProps,
    int64_t               in_NumLogDomains,
    HSTR_LOG_DOM         *in_pLogDomainIDs);

////////////////////////////////////////////////////////
// hStreams_AddBufferLogDomains
/// @ingroup hStreams_Source_MemMgmt
/// @brief  Create instances of the buffer in the logical domains specified
///         as parameters.
///
/// @param  in_Address
///         [in] Any address inside a buffer for which to create instantiations of
///         the buffer.
/// @param  in_NumLogDomains
///         [in] Number of logical domains for which to create instantiations of
///         the buffer.
/// @param  in_pLogDomainIDs
///         [in] Array of logical domains IDs for which to instantiate the buffer.
///         The array must not contain \c HSTR_SRC_LOG_DOMAIN.
///
/// @return If successful, \c hStreams_AddBufferLogDomains() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is NULL
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_pLogDomainIDs is NULL
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumLogDomains == 0</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_pLogDomainIDs array contains
///     duplicate entries.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if at least one entry in the
///     \c in_pLogDomainIDs array is not a valid logical domain ID.
/// @arg \c HSTR_NOT_FOUND if \c in_Address does not belong to any buffer.
/// @arg \c HSTR_RESULT_ALREADY_FOUND if \c in_pLogDomainIDs array contains at
///     least one ID of a logical domain for which the buffer is already
///     instantiated.
/// @arg HSTR_RESULT_OUT_OF_MEMORY if at least one of the instantiations cannot
///     be created due to memory exhaustion or unavailability of requested memory
///     kind, subject to the buffer's memory allocation policy specified in the
///     buffer's properties
///
/// @thread_safety Thread safe.
///
////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_AddBufferLogDomains(
    void            *in_Address,
    uint64_t         in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs);

////////////////////////////////////////////////////////
///
// hStreams_RmBufferLogDomains
/// @ingroup hStreams_Source_MemMgmt
/// @brief Deallocate buffer instantiations in the selected logical domains.
///
/// If <tt>-1</tt> is suppplied as the number of logical domains, \c in_pLogDomainIDs
/// must be \c NULL - all the instantiations of the buffer will be destroyed.
///
/// @note Removing all instantiations of the buffer does not result in removing
///     the logical buffer as such. That is to say, after removing all
///     instantiations of a buffer, the user is free to add new instantiations
///     to that buffer. In order to remove the logical buffer, a call to \c
///     hStreams_DeAlloc() is required.
///
/// @param  in_Address
///         [in] Any address inside a buffer for which to create instantiations of
///         the buffer.
/// @param  in_NumLogDomains
///         [in] Number of logical domains from which to delete the instantiations of
///         the buffer.
/// @param  in_pLogDomainIDs [in] Array of logical domains IDs from which to
///         delete the buffer's instantiations. The array must not contain
///         \c HSTR_SRC_LOG_DOMAIN.
///
/// @return If successful, \c hStreams_RmBufferLogDomains() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is \c NULL.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumLogDomains == 0</tt>
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumLogDomains < -1</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if <tt>in_NumLogDomains == -1 &&
///     in_pLogDomainIDs != NULL</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if <tt>in_NumLogDomains != -1 &&
///     in_pLogDomainIDs == NULL</tt>
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_pLogDomainIDs array contains
///     duplicate entries.
/// @arg \c HSTR_RESULT_DOMAIN_OUT_OF_RANGE if any entry in the \c in_pLogDomainIDs
///     array is not a valid logical domain ID.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_Address doesn't refer to a valid buffer.
/// @arg \c HSTR_RESULT_NOT_FOUND if the buffer doesn't have an instantiation for
///     any of the selected logical domains.
///
////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_RmBufferLogDomains(
    void            *in_Address,
    int64_t          in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs);

/////////////////////////////////////////////////////////
///
// hStreams_DeAlloc
/// @ingroup hStreams_Source_MemMgmt
/// @brief Destroy the buffer and remove all its instantiations.
///
/// @param  in_Address
///         [in] Any address inside a buffer to destroy.
///
///
/// @return If successful, \c hStreams_DeAlloc() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is \c NULL.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_Address doesn't refer to a valid buffer.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_DeAlloc(
    void               *in_Address);

/////////////////////////////////////////////////////////
///
// hStreams_GetBufferNumLogDomains
/// @ingroup hStreams_Source_MemMgmt
/// @brief Return the number of logical domains or which the buffer has been
///        instantiated.
///
/// @note The value written to \c out_pNumLogDomains does not count the implicit
///     instantiation of the buffer for \c HSTR_SRC_LOG_DOMAIN.
///
/// @param  in_Address
///         [in] Any address inside a buffer for which to create instantiations of
///         the buffer.
/// @param  out_pNumLogDomains
///         [out] Number of logical domains the buffer has been instantiated for.
///
/// @return If successful, \c hStreams_GetBufferNumLogDomains() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pNumLogDomains is \c NULL.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_Address doesn't refer to a valid buffer.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetBufferNumLogDomains(
    void                *in_Address,
    uint64_t            *out_pNumLogDomains);

/////////////////////////////////////////////////////////
///
// hStreams_GetBufferLogDomains
/// @ingroup hStreams_Source_MemMgmt
/// @brief Return a list of logical domains for which the buffer is instantiated.
///
/// < what happens when buffer too small >
///
/// @param  in_Address
///         [in] Source proxy address anywhere in a buffer.
/// @param  in_NumLogDomains
///         [in] Number of entries to write to the out_pLogDomains array.
/// @param  out_pLogDomains
///         [out] List of logical domains' IDs the buffer is instantiated.
///         At most in_NumLogDomains entries will be written.
/// @param  out_pNumLogDomains
///         [out] Number of logical domains the buffer has been instantiated for.
///         This doesn't include the implicit instantiation for \c HSTR_SRC_LOG_DOMAIN.
///
/// @return If successful, \c hStreams_GetBufferNumLogDomains() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if hStreams had not been initialized prior
///     to this call.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pLogDomains is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_pNumLogDomains is \c NULL.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_Address doesn't refer to a valid buffer.
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if <tt>in_NumLogDomains == 0</tt>.
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if \c in_NumLogDomains is less than the
///         actual number of logical domains where buffer is instantiated, not
///         counting \c HSTR_SRC_LOG_DOMAIN.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetBufferLogDomains(
    void                *in_Address,
    uint64_t             in_NumLogDomains,
    HSTR_LOG_DOM        *out_pLogDomains,
    uint64_t            *out_pNumLogDomains);

/////////////////////////////////////////////////////////
///
// hStreams_GetBufferProps
/// @ingroup hStreams_Source_MemMgmt
/// @brief Returns buffer properties associated with a buffer.
///
/// This API returns data for buffers of all types, including those not allocated with Alloc1Ex.
///
/// @param  in_Address
///         [in] Source proxy address anywhere in a buffer.
/// @param  out_BufferProps
///         [out] Buffer properties.
///
/// @return If successful, \c hStreams_GetBufferProps() returns
///     \c HSTR_RESULT_SUCCESS. Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_INITIALIZED if \c hStreams had not been initialized
///         properly.
/// @arg \c HSTR_RESULT_NULL_PTR if \c in_Address is \c NULL.
/// @arg \c HSTR_RESULT_NULL_PTR if \c out_BufferProps is \c NULL.
/// @arg \c HSTR_RESULT_NOT_FOUND if \c in_Address does not belong to any buffer.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetBufferProps(
    void                *in_Address,
    HSTR_BUFFER_PROPS   *out_BufferProps);

/////////////////////////////////////////////////////////
///
// hStreams_GetLastError
/// @ingroup hStreams_Source_Errors
/// @brief Get the last error.
///
/// @return Last recorded HSTR_RESULT (different than HSTR_SUCCESS)
///
/// @thread_safety Thread safe. Last error is recorded across all threads accessing the hStreams library API.
///
DllAccess HSTR_RESULT
hStreams_GetLastError();

/////////////////////////////////////////////////////////
///
// hStreams_ClearLastError
/// @ingroup hStreams_Source_Errors
/// @brief Clear the last hStreams error across.
///
/// @return void
///
/// @thread_safety Thread safe. Last error is recorded across all threads accessing the hStreams library API.
///
/////////////////////////////////////////////////////////
DllAccess void
hStreams_ClearLastError();

/////////////////////////////////////////////////////////
///
// hStreams_Cfg_SetLogLevel
/// @ingroup hStreams_Configuration
/// @brief Set a logging level for the hetero-streams library.
/// @param  in_loglevel
///         [in] The level at which to start reporting messages
///
/// @note Adjusting the logging level is only permitted \e outside the
///     intialization-finalization cycle for the hetero-streams library. A
///     value that is set before the first call to any of the intialization
///     functions is used until the finalization of the library.
///
/// @return If successful, \c hStreams_Cfg_SetLogLevel() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_PERMITTED if the hetero-streams library has been
///     already initialized
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the input argument value does not correpond to
///     a valid logging level
///
/// @thread_safety Not thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Cfg_SetLogLevel(
    HSTR_LOG_LEVEL   in_loglevel);

/////////////////////////////////////////////////////////
///
// hStreams_Cfg_SetLogInfoType
/// @ingroup hStreams_Configuration
/// @brief Set a bitmask of message categories that the library should emit.
/// @param  in_info_type_mask
///         [in] A bitmask filter to apply to the logging messages.
///
/// Hetero-streams logging mechanism categorises the messages that it can
/// produce. User can apply a filter to the messages that will be produced,
/// based on the message's information type. This filter takes the form of a
/// bitmask with meaning of individual bits defined by the values of the \c
/// HSTR_INFO_TYPE enumerated type.
///
/// @note Adjusting the message filter is only permitted \e outside the
///     intialization-finalization cycle for the hetero-streams library. A
///     value that is set before the first call to any of the intialization
///     functions is used until the finalization of the library.
///
/// @return If successful, \c hStreams_Cfg_SetLogInfoType() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_PERMITTED if the hetero-streams library has been
///     already initialized
///
/// @thread_safety Not thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Cfg_SetLogInfoType(
    uint64_t        in_info_type_mask);

/////////////////////////////////////////////////////////
///
// hStreams_Cfg_SetMKLInterface
/// @ingroup hStreams_Configuration
/// @brief Choose used MKL interface version
///
/// @return If successful, \c hStreams_Cfg_SetMKLInterface() returns \c HSTR_RESULT_SUCCESS.
///     Otherwise, it returns one of the following errors:
/// @arg \c HSTR_RESULT_NOT_PERMITTED if the hetero-streams library has been
///     already initialized
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if the input argument value does not correpond to
///     a valid MKL interface
///
/// @thread_safety Not thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Cfg_SetMKLInterface(
    HSTR_MKL_INTERFACE in_MKLInterface);

/////////////////////////////////////////////////////////
///
// hStreams_SetOptions
/// @ingroup hStreams_Configuration
/// @brief Configure user parameters by setting hStreams Options
///
/// @param  in_options - see the definition of HSTR_OPTIONS
///         [in] HSTR_OPTIONS
///
/// For more details about HSTR_OPTIONS please go directly
///  to HSTR_OPTIONS documentation
///
/// @return HSTR_RESULT_SUCCESS if setting options is successful
///
/// @return HSTR_RESULT_NULL_PTR if in_options is NULL.
///
/// @return HSTR_RESULT_INCONSISTENT_ARGS if options are inconsistent.
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_SetOptions(const HSTR_OPTIONS *in_options);

/////////////////////////////////////////////////////////
///
// hStreams_SetLibrariesToLoad
/// @ingroup hStreams_Configuration
/// @brief Set libraries to be loaded during initialization of hStreams for
///     given \c HSTR_ISA_TYPE
///
/// @param in_isaType
///         [in] ISA type for which given libraries will be loaded during
///         initialization
///
/// @param in_numLibNames
///         [in] Number of libraries names passed through \c in_ppLibNames
///
/// @param in_ppLibNames
///         [in] Names of libraries to be loaded
///
/// @param in_pLibFlags
///         [in] Flags to be used for loading libraries
///         Setting in_pLibFlags to NULL causes the default lib flags to be
///         used for loading all of libraries.
///         Setting in_pLibFlags other than NULL on host is not supported.
///
/// @return If successful, \c hStreams_SetLibrariesToLoad() returns \c
/// HSTR_RESULT_SUCCESS.
/// Otherwise, it returns on of the following errors:
/// @arg \c HSTR_RESULT_NOT_PERMITTED if the heterostreams library has been
///     already initialized
/// @arg \c HSTR_RESULT_OUT_OF_RANGE if in_isaType is not valid one or is not supported
///     or if in_NumLibNames == 0
/// @arg \c HSTR_RESULT_NULL_PTR if in_ppLibNames is NULL
/// @arg \c HSTR_RESULT_INCONSISTENT_ARGS if in_ppLibNames contain NULL entries
/// @arg \c HSTR_RESULT_NOT_IMPLEMENTED if in_isaType == HSTR_ISA_x86_64 and in_pLibFlags != NULL
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_SetLibrariesToLoad(
    HSTR_ISA_TYPE   in_isaType,
    uint32_t        in_numLibNames,
    char          **in_ppLibNames,
    int            *in_pLibFlags);

/////////////////////////////////////////////////////////
///
// hStreams_GetCurrentOptions
/// @ingroup hStreams_Configuration
/// @brief Query user parameters by getting hStreams Options
///
/// hStreams_GetCurrentOptions() copies the current collection of hStreams options from the hStreams library
/// to the buffer provided in a thread safe manner.
///
/// @param  pCurrentOptions
///         [out] The buffer that will receive the current options from the hStreams library.  Must be non-NULL or
///         hStreams_GetCurrentOptions() returns HSTR_RESULT_OUT_OF_RANGE.
///
/// @param  buffSize
///         [in] Indicates the size of the buffer that pCurrentOptions points to.  Must be greater than or equal to
///         sizeof(HSTR_OPTIONS) of hStreams_GetCurrentOptions() returns HSTR_RESULT_OUT_OF_RANGE.
///
/// For more details about HSTR_OPTIONS please go directly
///  to HSTR_OPTIONS documentation
///
/// @return HSTR_RESULT_SUCCESS if getting options is successful
///
/// @return HSTR_RESULT_OUT_OF_RANGE if pCurrentOptions is NULL, or buffSize is less than sizeof(HSTR_OPTIONS)
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetCurrentOptions(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize);

/////////////////////////////////////////////////////////
///
// hStreams_VersionStringLen
/// @ingroup hStreams_Utils
/// @brief Report the length of the version string, including the null termination character
///
/// @param  out_pVersionStringLen
///         [out] The length of the version string, including the null termination character
///
/// @return HSTR_RESULT_NULL_PTR if out_pVersionStringLen is NULL
/// @return HSTR_RESULT_SUCCESS
///
/// @thread_safety Thread safe.
///
//////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_GetVersionStringLen(uint32_t *out_pVersionStringLen);

/////////////////////////////////////////////////////////
///
// hStreams_Version
/// @ingroup hStreams_Utils
/// @brief Report hStreams version info to buffer
///
/// @param  buff
///         [out] The buffer that will receive the version of the hStreams library.
///
/// @param  buffLength
///         [in] The length of the buff parameter.  hStreamsVersion() copies version data
///         upto buffLength bytes to buff.
///
/// @return HSTR_RESULT_SUCCESS
///
/// @return HSTR_RESULT_NULL_PTR if the buffer argument is a NULL pointer
///
/// @return HSTR_RESULT_BUFF_TOO_SMALL when the buffer is too small.
///
/// @thread_safety Thread safe for invocations with different buffer arguments.
///
/// The version string is in format MAJOR.MINOR[.MICRO]. The MICRO-part is ommited if
/// MICRO == 0.
///
//////////////////////////////////////////////////////////
DllAccess HSTR_RESULT
hStreams_Version(char *buff, uint32_t buffLength);

/////////////////////////////////////////////////////////
/// @deprecated This function has been deprecated in favor of
///     hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().
//////////////////////////////////////////////////////////
HSTR_DEPRECATED("hStreams_GetVerbose() has been deprecated. "
                "Please refer to hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().")
DllAccess uint32_t
hStreams_GetVerbose();

/////////////////////////////////////////////////////////
/// @deprecated This function has been deprecated in favor of
///     hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().
//////////////////////////////////////////////////////////
HSTR_DEPRECATED("hStreams_SetVerbose() has been deprecated. "
                "Please refer to hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().")
DllAccess HSTR_RESULT
hStreams_SetVerbose(int target_verbosity);

/////////////////////////////////////////////////////////
///
// hStreams_ResultGetName
/// @ingroup hStreams_Utils
/// @brief Get HSTR_RESULT name
///
/// @param  in_olr
///         [in] HSTR_RESULT
///
/// @return Name string
///
/// @thread_safety Thread safe.
///
/////////////////////////////////////////////////////////
DllAccess const char *hStreams_ResultGetName(
    HSTR_RESULT in_olr);


/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_ISSET().
static inline uint64_t HSTR_CPU_MASK_ISSET(int bitNumber, const HSTR_CPU_MASK cpu_mask)
{
    if ((size_t)bitNumber < sizeof(HSTR_CPU_MASK) * 8) {
        return ((cpu_mask)[bitNumber / 64] & (((uint64_t)1) << (bitNumber % 64)));
    }
    return 0;
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_SET().
static inline void HSTR_CPU_MASK_SET(int bitNumber, HSTR_CPU_MASK cpu_mask)
{
    if ((size_t)bitNumber < sizeof(HSTR_CPU_MASK) * 8) {
        ((cpu_mask)[bitNumber / 64] |= (((uint64_t)1) << (bitNumber % 64)));
    }
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_ZERO().
static inline void HSTR_CPU_MASK_ZERO(HSTR_CPU_MASK cpu_mask)
{
    memset(cpu_mask, 0, sizeof(HSTR_CPU_MASK));
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_AND().
static inline void HSTR_CPU_MASK_AND(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] & src2[i];
    }
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_XOR().
static inline void HSTR_CPU_MASK_XOR(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] ^ src2[i];
    }
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_OR().
static inline void HSTR_CPU_MASK_OR(HSTR_CPU_MASK dst, const HSTR_CPU_MASK src1, const HSTR_CPU_MASK src2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(dst[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        dst[i] = src1[i] | src2[i];
    }
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_COUNT().
static inline int HSTR_CPU_MASK_COUNT(const HSTR_CPU_MASK cpu_mask)
{
    int cnt = 0;
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(cpu_mask[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        int cnt_i = 0;
        uint64_t n = cpu_mask[i];

        for (; n; n >>= 1) {
            cnt_i += n & 1l;
        }

        cnt += cnt_i;
    }
    return cnt;
}

/// @ingroup hStreams_CPUMASK
/// @brief Roughly equivalent to CPU_EQUAL().
static inline int HSTR_CPU_MASK_EQUAL(const HSTR_CPU_MASK cpu_mask1, const HSTR_CPU_MASK cpu_mask2)
{
    const unsigned int loopIterations = sizeof(HSTR_CPU_MASK) / sizeof(cpu_mask1[0]);

    for (unsigned int i = 0; i < loopIterations; ++i) {
        if (cpu_mask1[i] != cpu_mask2[i]) {
            return 0;
        }
    }
    return 1;
}

#ifndef _WIN32
/// @ingroup hStreams_CPUMASK
/// @brief Utility function to translate from cpu_set* to COI_CPU_MASK.
static inline void HSTR_CPU_MASK_XLATE(HSTR_CPU_MASK dest, const cpu_set_t *src)
{
    HSTR_CPU_MASK_ZERO(dest);
    for (unsigned int i = 0; i < sizeof(HSTR_CPU_MASK) / sizeof(dest[0]); ++i) {
        for (unsigned int j = 0; j < 64; ++j) {
            if (CPU_ISSET(i * 64 + j, src)) {
                dest[i] |= ((uint64_t)1) << j;
            }
        }
    }
}

/// @ingroup hStreams_CPUMASK
/// @brief  Utility function to translate from COI_CPU_MASK to cpu_set*.
static inline void HSTR_CPU_MASK_XLATE_EX(cpu_set_t *dest, const HSTR_CPU_MASK src)
{
    CPU_ZERO(dest);
    for (unsigned int i = 0; i < sizeof(HSTR_CPU_MASK) / sizeof(src[0]); ++i) {
        const uint64_t cpu_mask = src[i];

        for (unsigned int j = 0; j < 64; ++j) {
            const uint64_t bit = ((uint64_t)1) << j;

            if (bit & cpu_mask) {
                CPU_SET(i * 64 + j, dest);
            }
        }
    }
}

#endif //_WIN32

#define CHECK_HSTR_RESULT(func)                                               \
    {                                                                         \
        HSTR_RESULT hret = HSTR_RESULT_SUCCESS;                               \
        hret = func;                                                          \
        if (hret != HSTR_RESULT_SUCCESS) {                                    \
            printf("%s returned %s.\n", #func, hStreams_ResultGetName(hret)); \
            return hret;                                                      \
        }                                                                     \
    }

#ifdef __cplusplus
}
#endif

#endif
