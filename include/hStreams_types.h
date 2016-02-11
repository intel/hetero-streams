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

#ifndef __HSTR_TYPES_H__

#define __HSTR_TYPES_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "hStreams_version.h"

#ifndef _WIN32
// GNU supports messages in deprecated attribute since 4.5
#if __GNUC__ > 4 || \
        (__GNUC__ == 4 && __GNUC_MINOR__ > 4)
#define HSTR_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else // GNU >= 4.5
#define HSTR_DEPRECATED(msg) __attribute__((deprecated))
#endif // GNU >= 4.5
#else // _WIN32
#define HSTR_DEPRECATED(msg) __declspec(deprecated(msg))
#endif // _WIN32

#ifndef _WIN32
#include <stdint.h>
#include <wchar.h>
#else
#if (_MSC_VER >= 1700) || defined(__INTEL_COMPILER)
#include <stdint.h>
#else
#ifndef _STDINT
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define _STDINT
#endif //_STDINT
#endif //_MSC_VER
#endif //_WIN32

#endif //DOXYGEN_SHOULD_SKIP_THIS

#define HSTR_BUFFER_PROPS_INITIAL_VALUES {        \
        HSTR_MEM_TYPE_NORMAL,                     \
        HSTR_MEM_ALLOC_PREFERRED,                 \
        HSTR_BUF_PROP_SRC_PINNED}

#define HSTR_BUFFER_PROPS_INITIAL_VALUES_EX {     \
        HSTR_MEM_TYPE_NORMAL,                     \
        HSTR_MEM_ALLOC_PREFERRED,                 \
        0}

/////////////////////////////////////////////////////////
// Doxygen settings
/// @file include/hStreams_types.h
///
/// @defgroup hStreams_Types hStreams Types
/// @{
// TODO: Description for this group
//
/////////////////////////////////////////////////////////

// Begin defines and enumerated types

/// @brief For managing overlap of cpu masks of partitions
typedef int HSTR_OVERLAP_TYPE;

/// @brief Possible values of \c HSTR_OVERLAP_TYPE
typedef enum {
    /// No overlap among streams: intersection is null
    HSTR_NO_OVERLAP = 0,

    /// Exact overlap among streams: non-null and intersection == union
    HSTR_EXACT_OVERLAP,

    /// Partial overlap among streams: none of the above
    HSTR_PARTIAL_OVERLAP

} HSTR_OVERLAP_TYPE_VALUES;

/// @brief Type that is returned from hStream functions
typedef int HSTR_RESULT;

/// @brief Possible values of \c HSTR_RESULT
typedef enum {
    /// Successful completion
    HSTR_RESULT_SUCCESS = 0,

    /// A remote error (e.g. with COI) resulted in early termination
    HSTR_RESULT_REMOTE_ERROR,

    /// Results are not valid due to lack of successful initialization - may not be any MIC cards
    HSTR_RESULT_NOT_INITIALIZED,

    /// The object that an input key was being used to look up was not found.
    /// For example, the in_LogStream was not found to have a corresponding hStream object
    HSTR_RESULT_NOT_FOUND,

    /// The object that an input key was being used to look up was already found
    HSTR_RESULT_ALREADY_FOUND,

    /// The given input type does not exist or has a bad value
    HSTR_RESULT_OUT_OF_RANGE,

    /// The given domain does not exist
    HSTR_RESULT_DOMAIN_OUT_OF_RANGE,

    /// The given CPU mask is not within range
    HSTR_RESULT_CPU_MASK_OUT_OF_RANGE,

    /// One or more domains do not have adequate memory
    HSTR_RESULT_OUT_OF_MEMORY,

    /// Specifying the given input type would violate an invariant
    HSTR_RESULT_INVALID_STREAM_TYPE,

    /// Resource allocation overlaps when it should not
    HSTR_RESULT_OVERLAPPING_RESOURCES,

    /// The requested device is not available
    HSTR_RESULT_DEVICE_NOT_INITIALIZED,

    /// Bad name, e.g. null function name
    HSTR_RESULT_BAD_NAME,

    /// Too many function arguments
    HSTR_RESULT_TOO_MANY_ARGS,

    /// Event wait time out
    HSTR_RESULT_TIME_OUT_REACHED,

    /// Event canceled
    HSTR_RESULT_EVENT_CANCELED,

    /// The arguments passed to an hStreams function are inconsistent.
    HSTR_RESULT_INCONSISTENT_ARGS,

    /// The given buffer is too small
    HSTR_RESULT_BUFF_TOO_SMALL,

    /// The description of a memory operand is inconsistent with past actions
    HSTR_RESULT_MEMORY_OPERAND_INCONSISTENT,

    /// An argument is NULL that shouldn't be
    HSTR_RESULT_NULL_PTR,

    /// Internal error
    HSTR_RESULT_INTERNAL_ERROR,

    /// Any resource other than memory exhausted, e.g. number of threads
    HSTR_RESULT_RESOURCE_EXHAUSTED,

    /// Not implemented yet.
    HSTR_RESULT_NOT_IMPLEMENTED,

    /// Requested operation is not allowed.
    HSTR_RESULT_NOT_PERMITTED,

    /// Dummy last entry
    HSTR_RESULT_SIZE

} HSTR_RESULT_VALUES;
// If this is changed, be sure to update corresponding array of names


/// @brief Underlying type large enough to encompass any of HSTR_INFO_TYPE_VALUES
typedef int64_t HSTR_INFO_TYPE;

/// @brief Message type categories
typedef enum {
    /// @brief Function invocation traces
    HSTR_INFO_TYPE_TRACE                    = (1ULL << 5),
    /// @brief Sink-side kernel invocations
    HSTR_INFO_TYPE_SINK_INVOKE              = (1ULL << 6),
    /// @brief Memory-related messages
    HSTR_INFO_TYPE_MEM                      = (1ULL << 7),
    /// @brief Messages related to synchronization events
    HSTR_INFO_TYPE_SYNC                     = (1ULL << 8),
    /// @brief Miscallenous messages
    HSTR_INFO_TYPE_MISC                     = (1ULL << 9),

    // start deprecated placeholders
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    DEPRECATED_HSTR_INFO_TYPE_ALWAYSEMIT = 0,
    DEPRECATED_HSTR_INFO_TYPE_TRACE = 1,
    DEPRECATED_HSTR_INFO_TYPE_INVOKE = 2,
    DEPRECATED_HSTR_INFO_TYPE_DEPS = 4,
    DEPRECATED_HSTR_INFO_TYPE_GENERAL = 7,
    DEPRECATED_HSTR_INFO_TYPE_AND16 = (1ULL << 4),
#endif // DOXYGEN_SHOULD_SKIP_THIS
    // end deprecated placeholders
} HSTR_INFO_TYPE_VALUES;

/// @deprecated HSTR_INFO_TYPE_ALWAYSEMIT has been deprecated
/// @sa HSTR_INFO_TYPE_VALUES
HSTR_DEPRECATED("HSTR_INFO_TYPE_ALWAYSEMIT has been deprecated. Please refer to HSTR_INFO_TYPE_VALUES.")
const HSTR_INFO_TYPE_VALUES HSTR_INFO_TYPE_ALWAYSEMIT = DEPRECATED_HSTR_INFO_TYPE_ALWAYSEMIT;

/// @deprecated HSTR_INFO_TYPE_INVOKE has been deprecated
/// @sa HSTR_INFO_TYPE_VALUES
HSTR_DEPRECATED("HSTR_INFO_TYPE_INVOKE has been deprecated. Please refer to HSTR_INFO_TYPE_VALUES.")
const HSTR_INFO_TYPE_VALUES HSTR_INFO_TYPE_INVOKE = DEPRECATED_HSTR_INFO_TYPE_INVOKE;

/// @deprecated HSTR_INFO_TYPE_DEPS has been deprecated
/// @sa HSTR_INFO_TYPE_VALUES
HSTR_DEPRECATED("HSTR_INFO_TYPE_DEPS has been deprecated. Please refer to HSTR_INFO_TYPE_VALUES.")
const HSTR_INFO_TYPE_VALUES HSTR_INFO_TYPE_DEPS = DEPRECATED_HSTR_INFO_TYPE_DEPS;

/// @deprecated HSTR_INFO_TYPE_GENERAL has been deprecated
/// @sa HSTR_INFO_TYPE_VALUES
HSTR_DEPRECATED("HSTR_INFO_TYPE_GENERAL has been deprecated. Please refer to HSTR_INFO_TYPE_VALUES.")
const HSTR_INFO_TYPE_VALUES HSTR_INFO_TYPE_GENERAL = DEPRECATED_HSTR_INFO_TYPE_GENERAL;

/// @deprecated HSTR_INFO_TYPE_AND16 has been deprecated
/// @sa HSTR_INFO_TYPE_VALUES
HSTR_DEPRECATED("HSTR_INFO_TYPE_AND16 has been deprecated. Please refer to HSTR_INFO_TYPE_VALUES.")
const HSTR_INFO_TYPE_VALUES HSTR_INFO_TYPE_AND16 = DEPRECATED_HSTR_INFO_TYPE_AND16;

typedef enum HSTR_SEVERITY {
    // start deprecated placeholders
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    DEPRECATED_HSTR_SEVERITY_INFO,
    DEPRECATED_HSTR_SEVERITY_WARNING,
    DEPRECATED_HSTR_SEVERITY_ERROR,
    DEPRECATED_HSTR_SEVERITY_FATAL_ERROR
#endif // DOXYGEN_SHOULD_SKIP_THIS
    // end deprecated placeholders
} HSTR_SEVERITY;

/// @deprecated HSTR_SEVERITY_INFO has been deprecated
/// @sa HSTR_LOG_LEVEL
HSTR_DEPRECATED("HSTR_SEVERITY_INFO has been deprecated. Please refer to HSTR_LOG_LEVEL.")
const HSTR_SEVERITY HSTR_SEVERITY_INFO = DEPRECATED_HSTR_SEVERITY_INFO;

/// @deprecated HSTR_SEVERITY_WARNING has been deprecated
/// @sa HSTR_LOG_LEVEL
HSTR_DEPRECATED("HSTR_SEVERITY_WARNING has been deprecated. Please refer to HSTR_LOG_LEVEL.")
const HSTR_SEVERITY HSTR_SEVERITY_WARNING = DEPRECATED_HSTR_SEVERITY_WARNING;

/// @deprecated HSTR_SEVERITY_ERROR has been deprecated
/// @sa HSTR_LOG_LEVEL
HSTR_DEPRECATED("HSTR_SEVERITY_ERROR has been deprecated. Please refer to HSTR_LOG_LEVEL.")
const HSTR_SEVERITY HSTR_SEVERITY_ERROR = DEPRECATED_HSTR_SEVERITY_ERROR;

/// @deprecated HSTR_SEVERITY_FATAL_ERROR has been deprecated
/// @sa HSTR_LOG_LEVEL
HSTR_DEPRECATED("HSTR_SEVERITY_FATAL_ERROR has been deprecated. Please refer to HSTR_LOG_LEVEL.")
const HSTR_SEVERITY HSTR_SEVERITY_FATAL_ERROR = DEPRECATED_HSTR_SEVERITY_FATAL_ERROR;


/// @brief Underlying type large enough to encompass any of HSTR_LOG_LEVEL_VALUES
typedef int32_t HSTR_LOG_LEVEL;

/// @brief Message type categories
typedef enum {
    HSTR_LOG_LEVEL_NO_LOGGING,
    HSTR_LOG_LEVEL_FATAL_ERROR,
    HSTR_LOG_LEVEL_ERROR,
    HSTR_LOG_LEVEL_WARN,
    HSTR_LOG_LEVEL_LOG,
    HSTR_LOG_LEVEL_DEBUG1,
    HSTR_LOG_LEVEL_DEBUG2,
    HSTR_LOG_LEVEL_DEBUG3,
    HSTR_LOG_LEVEL_DEBUG4
} HSTR_LOG_LEVEL_VALUES;


/// @brief This is the type associated with hStream dependence policies
typedef int HSTR_DEP_POLICY;

/// @brief Possible values of \c HSTR_DEP_POLICY
typedef enum {
    /// Everything submitted to an hStream depends on everything before it
    HSTR_DEP_POLICY_CONSERVATIVE = 0,

    /// Dependendencies are based on the existence of RAW, WAW, WAE to the same buffer
    HSTR_DEP_POLICY_BUFFERS,

    /// Dependencies ignored, for perf debug testing only; must be last
    HSTR_DEP_POLICY_NONE,

    /// One past the max supported value; = to # of supported values
    HSTR_DEP_POLICY_SIZE

} HSTR_DEP_POLICY_VALUES;

///@brief Type associated with hStream's KMP affinity policy
typedef int HSTR_KMP_AFFINITY;

///@brief Possible values of \c HSTR_KMP_AFFINITY
typedef enum {
    /// Balanced
    /// Since there are as many OpenMP threads as there are HW threads reserved
    /// for the stream, this option works exactly the same as \c HSTR_KMP_AFFINITY_COMPACT.
    HSTR_KMP_AFFINITY_BALANCED = 0,

    /// Compact
    /// This mode associates openmp threads to cores within the same
    /// processor first then moves to adjacent processor.
    HSTR_KMP_AFFINITY_COMPACT,

    /// Scatter
    /// This mode associates OpenMP threads to all assigned processors, spreading them across cores first.
    /// That is, the first OpenMP thread will be affinitized to the first available HW thread of the first core,
    /// the second OpenMP thread will be affinitized to the first available HW thread of the second core.
    /// If \c N is the number of cores assigned to a given stream, \c N-th OpenMP thread will be affinitized
    /// to the second available HW thread of the first core and so on.
    HSTR_KMP_AFFINITY_SCATTER,

    /// One past the max supported value; = to # of supported values
    HSTR_KMP_AFFINITY_SIZE

} HSTR_KMP_AFFINITY_VALUES;

/// @brief Type associated with hStream OpenMP handling
/// FIXME: This will be changing in the transition to support other threading runtimes
typedef int HSTR_OPENMP_POLICY;

/// @brief Possible values of \c HSTR_KMP_AFFINITY
typedef enum {
    /// OpenMP handled entirely by user, without involvement from hStreams
    /// Warning: Unless the user does explicit affinitization, this is likely to lead to
    ///  having all partitions oversubscribe the first partition that gets defined
    HSTR_OPENMP_ON_DEMAND = 0,

    /// Set up OpenMP parallel region per partition and affinitize its threads upon StreamCreate
    HSTR_OPENMP_PRE_SETUP = 1,

    /// One past the max supported value; = to # of supported values
    HSTR_OPENMP_POLICY_SIZE

} HSTR_OPENMP_POLICY_VALUES;

/// @brief Type associated with hStream physical memory types
/// These are consecutive integers, NOT mask values
typedef int HSTR_MEM_TYPE;

/// @brief Possible values of \c HSTR_MEM_TYPE
typedef enum {
    /// Unspecified, could be any
    HSTR_MEM_TYPE_ANY = -1,

    /// Normal DRAM
    HSTR_MEM_TYPE_NORMAL = 0,

    /// High bandwidth DRAM
    HSTR_MEM_TYPE_HBW,

    /// One past the max supported value; = to # of supported values
    HSTR_MEM_TYPE_SIZE

} HSTR_MEM_TYPE_VALUES;

/// @brief Type associated with direction of data transfer
typedef int HSTR_XFER_DIRECTION;

/// @brief Possible values of \c HSTR_XFER_DIRECTION
typedef enum {
    /// Sink (stream endpoint) to source (where command issued from)
    HSTR_SINK_TO_SRC = 0,

    /// Source (where command issued from) to sink (stream endpoint)
    HSTR_SRC_TO_SINK = 1
} HSTR_XFER_DIRECTION_VALUES;

/// @brief This type encapsulates the COI_ISA_TYPE, to enable building hStreams
///  on something other than COI in the future.
/// It is used to indicate the ISA for enumerated domains
typedef int HSTR_ISA_TYPE;

/// @brief Possible values of \c HSTR_ISA_TYPE
typedef enum {
    HSTR_ISA_INVALID = 0,        ///< Represents an invalid ISA.
    HSTR_ISA_x86_64,             ///< The ISA for an x86_64 host engine.
    HSTR_ISA_MIC,                ///< Special value used to represent any device
    ///< in the Intel(R) Many Integrated Core
    ///< architecture family.
    HSTR_ISA_KNF,                ///< ISA for L1OM devices.
    HSTR_ISA_KNC,                ///< ISA for K1OM devices.
    HSTR_ISA_KNL                 ///<
} HSTR_ISA_TYPE_VALUES;

/// @brief Type associated with mask of flags for buffers
typedef int HSTR_BUFFER_PROP_FLAGS;

/// @brief Possible values of \c HSTR_BUFFER_PROP_FLAGS
typedef enum {
    /// Buffer instances should be aliased when their logical
    ///  domains are mapped to the same physical domain
    HSTR_BUF_PROP_ALIASED = 1,

    /// The instance associated with \c HSTR_SRC_LOG_DOMAIN is pinned when
    ///  buffer is created. Otherwise defer pinning until on-access demand.
    HSTR_BUF_PROP_SRC_PINNED = 2,

    /// When a new logical domain is added, an instantiation of
    ///  this buffer is automatically added for that log domain
    HSTR_BUF_PROP_INCREMENTAL = 4,

    /// The first touch of each instantiation of this buffer
    ///  is constrained to be performed by a thread that belongs
    ///  to the CPU set of its logical domain
    /// Functionality of this flag is not implemented yet, \c Alloc1DEx
    ///  return \c HSTR_RESULT_NOT_IMPLEMENTED if that flag is set.
    HSTR_BUF_PROP_AFFINITIZED = 8,

    /// First invalid value of bitmask. Value of last flag * 2.
    HSTR_BUF_PROP_INVALID_VALUE = 16
} HSTR_BUFFER_PROP_FLAGS_VALUES;

/// @brief Type associated with hStream memory allocation policy regarding
/// the behaviour when either:
///     a. the requested memory type has been exhausted on some node
///     b. the requested memory type does not even exist on some node
typedef int HSTR_MEM_ALLOC_POLICY;

/// @brief Possible values of \c HSTR_MEM_ALLOC_POLICY
typedef enum {
    /// Try to alloc specified memory type, mem_type is treated as HSTR_MEM_TYPE_ANY
    ///  when given memory type not available
    HSTR_MEM_ALLOC_PREFERRED = 0,

    /// Alloc only specified memory type, fail when such memory is not avaliable on
    ///  any of the specified logical domains.
    HSTR_MEM_ALLOC_STRICT,

    /// One past the max supported value; = to # of supported values
    HSTR_MEM_ALLOC_POLICY_SIZE
} HSTR_MEM_ALLOC_POLICY_VALUES;

typedef int HSTR_MKL_INTERFACE;
/// @brief Possible values of \c HSTR_MKL_INTERFACE
typedef enum {
    /// Use LP64 interface of Intel(R) MKL (MKL_INT size is 32b)
    HSTR_MKL_LP64 = 0,

    /// Use ILP64 interface of Intel(R) MKL (MKL_INT size is 64b)
    HSTR_MKL_ILP64,

    /// Don't load Intel(R) MKL libraries at all
    HSTR_MKL_NONE,

    /// One past the max supported value
    HSTR_MKL_INTERFACE_SIZE
} HSTR_MKL_INTERFACE_VALUES;

// End public enumerated types
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/// HSTR_LOG_STR, HSTR_LOG_DOM and HSTR_PHYS_DOM are aliases in order to
/// abstract the interfaces and hide the implementations from the client
/// to stress that the implementation of this datatype can change w/o notice.
/// It may be used for either physical or logical streams.
/// HSTR_LOG_STR is 64 bit, so that it can map from any 64b address to a
///  logical stream.

typedef uint64_t HSTR_LOG_STR;
typedef uint32_t HSTR_LOG_DOM;
typedef int32_t  HSTR_PHYS_DOM;

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Begin public function decl types

/* Prototype for hStreams emit message function. */
typedef int hStreams_EmitMessage_Prototype(HSTR_SEVERITY, const char *, const char *, ...);

/* Function pointer declaration pointing to a function that has same prototype for hStreams emit message function. */
typedef hStreams_EmitMessage_Prototype(*hStreams_EmitMessage_Prototype_Fptr);

/* Function pointer declaration pointing to a function that has same prototype for hStreams fatal error function. */
typedef void (*hStreams_FatalError_Prototype_Fptr)(int);

// End public function decl types
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Begin public struct types


/////////////////////////////////////////////////////////////////////
/// This type defines functions for the hStreams library to use
/// for error handling, and other future options.
// Warning: make sure SetOptions and GetOptions header file
//  descriptions are kept up to date.
typedef struct HSTR_OPTIONS {
    ///@cond internal_doc
    hStreams_EmitMessage_Prototype_Fptr _hStreams_EmitMessage;
    /* Note that the _hStreams_FatalError has the same prototype as exit(). */
    hStreams_FatalError_Prototype_Fptr  _hStreams_FatalError;
    /// @endcond
    /// @deprecated This option has been deprecated in favor of
    ///     hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().
    HSTR_DEPRECATED("HSTR_OPTIONS::verbose has been deprecated. "
                    "Please refer to hStreams_Cfg_SetLogLevel() and hStreams_Cfg_SetLogInfoType().")
    uint32_t               verbose;
    HSTR_KMP_AFFINITY      kmp_affinity;   ///< controls thread affinitization
    HSTR_DEP_POLICY        dep_policy;     ///< control deps only or data (default)
    uint32_t               phys_domains_limit; ///< max # of phys domains
    HSTR_OPENMP_POLICY     openmp_policy;  ///< controls OpenMP startup
    int                    time_out_ms_val;///< timeout for sync waits
    /* The following struct members allow the hstreams client application
       to explicitly define the collection of sink-side libraries that
       should be loaded on MIC sinks during initialization.
       By default, one library is determined at runtime for the client
        application.  See more comments near declaration of hStreams_Init(). */
    uint16_t               libNameCnt;  /* Both: libFlags[] and libNames[]
                                          should have libNameCnt elements if
                                          either is defined.  */
    int                   *libFlags;    /* setting libFlags to NULL causes the
                                         default COI lib flags to be used
                                         for loading all of libNames.
                                         Else, libFlags needs to have
                                         libNameCnt entries, and for all i:
                                         0 <= i < libNameCnt, libFlags[i]
                                         should explicitly define the lib
                                         flags to be used when loading the
                                         library in libNames[i]. */
    char                 **libNames;    /* there should be libNameCnt libNames. */

    /* The following struct members allow the hstreams client application
       to explicitly define the collection of sink-side libraries that
       should be loaded on host sinks during initialization.
       By default, one library is determined at runtime for the client
       application. See more comments near declaration of hStreams_Init(). */
    uint16_t libNameCntHost;           /* libNames_host[] should have libNameCnt elements if
                                            either is defined. */

    char **libNamesHost;               /* there should be libNameCnt_host libNames_host. */
} HSTR_OPTIONS;

/////////////////////////////////////////////////////////////////////
/// Additional properties when creating new buffer via Alloc1DEx.
/// Default properties are:
/// - Prefer to use normal DRAM.
/// - Not enable aliasing.
/// - The instance associated with HSTR_SRC_LOG_DOMAIN is pinned.
/// - Buffer will not be instantiated for logical domains created after its
///   allocation.
typedef struct HSTR_BUFFER_PROPS {
    /// Memory type
    HSTR_MEM_TYPE     mem_type;
    /// Memory allocation policy
    HSTR_MEM_ALLOC_POLICY  mem_alloc_policy;
    /// Bitmask. Allowed values are described in HSTR_BUFFER_PROP_FLAGS.
    uint64_t          flags;
} HSTR_BUFFER_PROPS;

// End public struct types
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// Begin public opaque types
/// @cond internal_doc

/////////////////////////////////////////////////////////////////////
/// This type imitates the COI_EVENT type.
/// This type defines the current use for monitoring asynchronous
/// events in the hStreams library.
typedef struct HSTR_EVENT {
    uint64_t opaque[2];
} HSTR_EVENT;

//////////////////////////////////////////////////////////////////////////////
///
/// This can be used to initialize an HSTR_EVENT to known invalid state.
/// This should be used if there's a lack of certainty that the event will
///   be initialized by the runtime.
/// Simply set the event to this value: HSTR_EVENT event = HSTR_EVENT_INITIALIZER;
// FIXME: The -1 only compiles with a late version of MPSS 3.5, not earlier
//  The use of 0 for now passes functional tests.
#define HSTR_EVENT_INITIALIZER { { 0, (uint64_t)-1 } }

/////////////////////////////////////////////////////////////////////
/// This type imitates the COI_CPU_MASK type.
/// It is used to provide an analog to unix cpu_set.  It supports
///  many more than 64 bits.
typedef uint64_t HSTR_CPU_MASK[16];

// End public opaque types
/// @endcond
/////////////////////////////////////////////////////////////////////

//End of hStreams_Types
/// @}
#endif
