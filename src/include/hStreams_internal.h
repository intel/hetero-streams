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

#ifndef __HSTR_INTERNAL_H__

#define __HSTR_INTERNAL_H__

/////////////////////////////////////////////////////////
// Doxygen settings
/// @file internal/hStreams_internal.h
/// @group HSInternal Internal data structures and functions
/// @{
/////////////////////////////////////////////////////////
#ifndef DOXYGEN_SHOULD_SKIP_THIS

// This version of hStreams only tested with version 2 of the COI Library API
#define COI_LIBRARY_VERSION 2

#include <hStreams_source.h>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <string>
#ifndef _WIN32
#include <unistd.h>
#include "hStreams_version.h"
#include <unistd.h>
/* Linux properly defines the PRI*64 macros in inttype.h when the following macro is defined: */
#define  __STDC_FORMAT_MACROS
/* Windows does not provide inttypes.h. */
#include <inttypes.h>
#else
#include <Windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "hStreams_locks.h"
#include "hStreams_atomic.h"
#include "hStreams_internal_types_common.h"
#include "hStreams_COIWrapper.h"
using namespace std;
#include <string.h>
#include <mkl.h>

#endif //DOXYGEN_SHOULD_SKIP_THIS

// Only relevant on Linux but who cares
#define HSTR_DEFAULT_SYMVER HSTREAMS_V

#define HSTR_STRINGIZE(arg) HSTR_STRINGIZE2(arg)
#define HSTR_STRINGIZE2(arg) #arg

#define __HSTR_VERSIONED_NAME2(symbol, sep, suffix) symbol##sep##suffix
#define __HSTR_VERSIONED_NAME(symbol, suffix) __HSTR_VERSIONED_NAME2(symbol, __, suffix)

#ifndef _WIN32
#define HSTR_EXPORT_IN_VERSION(ret_type, symbol, version)           \
    __asm__(".symver "                                              \
        HSTR_STRINGIZE(__HSTR_VERSIONED_NAME(symbol, __LINE__))     \
        "," HSTR_STRINGIZE(symbol) "@" HSTR_STRINGIZE(version));    \
    extern "C" ret_type __HSTR_VERSIONED_NAME(symbol, __LINE__)

#define HSTR_EXPORT_IN_DEFAULT_VERSION(ret_type, symbol)            \
    __asm__(".symver "                                              \
        HSTR_STRINGIZE(__HSTR_VERSIONED_NAME(symbol, __LINE__))     \
        "," HSTR_STRINGIZE(symbol) "@@"                             \
        HSTR_STRINGIZE(HSTR_DEFAULT_SYMVER));                       \
    extern "C" ret_type __HSTR_VERSIONED_NAME(symbol, __LINE__)
#else // _WIN32
// On windows we do NOT expose the non-default versions (we never did), so we
// just mangle the name
#define HSTR_EXPORT_IN_VERSION(ret_type, symbol, version)   \
    ret_type __HSTR_VERSIONED_NAME(symbol, __LINE__)

#define HSTR_EXPORT_IN_DEFAULT_VERSION(ret_type, symbol)   \
    extern "C" ret_type symbol
    // __pragma(comment(                                               \
    //     linker,                                                     \
    //     "/EXPORT:" HSTR_STRINGIZE(symbol)                           \
    //     "=" HSTR_STRINGIZE(__HSTR_VERSIONED_NAME(symbol, __LINE__)) \
    // ));                                                             \
    // extern "C" ret_type __HSTR_VERSIONED_NAME(symbol, __LINE__)
    // extern "C" ret_type symbol
    // __pragma(comment(linker, "/EXPORT:" HSTR_STRINGIZE(symbol) "=" HSTR_STRINGIZE(__HSTR_VERSIONED_NAME(symbol, __LINE__)) )); \
    // extern "C" ret_type __HSTR_VERSIONED_NAME(symbol, __LINE)
#endif // _WIN32


/**
 * Magic string to be consumed by \c hStreams_InitInVersion() and similar
 */
#define HSTR_MAGIC_PRE1_0_0_VERSION_STRING "pre1.0.0"

/**
 * Helper macro to increase the call count (i.e. how many APIs are currently being invoked
 */
#define HSTR_CORE_API_CALLCOUNTER() hStreams_ScopedAtomicCounter sac(hstr_proc.callCounter)

/* Linux follows the C99 standard and defines PRId64 and PRIx64 in inttypes.h (see above comments
   before #include <inttypes.h>).

   As windows does not even provide inttypes.h nor definitions for PRId64 and PRIx64, we define
   them here.  */
#if !defined(PRId64) && defined(_WIN32)
#define PRId64 "I64d"
#endif

#if !defined(PRIx64) && defined(_WIN32)
#define PRIx64 "I64x"
#endif

#if !defined(PRIu64) && defined(_WIN32)
#define PRIu64 "I64u"
#endif

#ifdef _WIN32
#   define mpss_hstreams_strdup _strdup
#   define mpss_hstreams_strtok strtok_s
#   define mpss_hstreams_stat   _stat
#else
#   define mpss_hstreams_strdup strdup
#   define mpss_hstreams_strtok strtok_r
#   define mpss_hstreams_stat   stat
#endif

#undef UNREFERENCED_PARAM

#ifndef UNREFERENCED_PARAM
#define UNREFERENCED_PARAM(P)  void* addr_ ##P = (void*) &P; addr_ ##P = addr_ ##P;
#endif

// Note that UNUSUED_ATTR means that it is "possibly" unused, not "definitely".
// This should compile out in release mode if indeed it is unused.
#ifndef _WIN32
#define UNUSED_ATTR __attribute__((unused))
#else
/* FIXME: Is there a better definition for UNUSED_ATTR on windows? */
#define UNUSED_ATTR /*for now, comment this out: __attribute__((unused))*/
#endif

/* Currently Symbol Versioning is enabled only on the Linux host*/
#ifndef HSTR_SYMBOL_VERSION
# if (!defined _WIN32) && (defined HSTR_SOURCE)
#   define HSTR_SYMBOL_VERSION( SYMBOL , VERSION ) SYMBOL ## VERSION
# else
#   define HSTR_SYMBOL_VERSION( SYMBOL , VERSION ) SYMBOL /* do not include ## VERSION */
# endif
#endif /* HSTR_SYMBOL_VERSION */

// MIC configuration
#define HSTR_MAX_THREADS 244
// May be more than what the actual platforms support, used for printing only
#define HSTR_THREADS_PER_CORE 4
#define HSTR_OS_START_ADJUST 1 // Threads to avoid are assumed to be contiguous
#define HSTR_OS_END_ADJUST 3

// Other
#define HSTR_BAD_DOMAIN -99

// The HSTR_ALIGN() macro forces variable allocation to be aligned per alignment factor X:
#ifdef _WIN32
#define HSTR_ALIGN(X) __declspec(align(X))
#else
#define HSTR_ALIGN(X) __attribute__((aligned(X)))
#endif

/* The C macro: HSTR_THUNK_FILE is defined in Makefile. */

// HSTR_STATIC_ASSERT is an instance of 'static assertion'.  See Google for more infromation.
// For example: http://www.drdobbs.com/compile-time-assertions/184401873
// Indicated that a static assertion allows catching bugs at compile time.
// Note that the constant expression, 'condition' will be evaluated by the compiler at compile time,
// and then, the compiler will abort compilation if the condition is false.  An example of its use is
// below.  See more notes there.
#define HSTR_STATIC_ASSERT( condition, name ) typedef char assert_failed_ ## name [ (condition) ? 1 : -1 ]


//Those pragmas are used to disable C4127 warning inside VS compiler.
//C4127 (Constant value in conditional expression) is caused
//by while(0) inside HSTR_RETURN macro.
#ifdef _WIN32
#define DISABLE_4127_WARNING __pragma(warning( push )) \
    __pragma(warning( disable : 4127 ))
#define ENABLE_4127_WARNING __pragma(warning( pop ))
#else
#define DISABLE_4127_WARNING
#define ENABLE_4127_WARNING
#endif


// For managing what type of dependence accounting is necessary for an operation
typedef enum DEP_TYPE {
    // Don't wait on preceeding computes, only on transfers
    IS_XFER,

    // Wait on preceeding computes
    IS_COMPUTE,

    // Wait on preceeding computes and all bufs
    IS_BARRIER,

    // Wait on nothing, just use the completion event
    NONE
} DEP_TYPE;

// Main data structure that spans streams
typedef struct hStreamHostProcess {
    // callCounter gives a count of the number of calls into the core hStreams library
    // functions.  So, for example, suppose after initialization, 2 threads allocate two different
    // buffers at the same time using hStreams_Alloc1D(), then, during those two calls,
    // callCounter will equal 2.  After both of the function calls exit, callCounter is
    // returned to 0.   callCounter is needed to allow hStreams_Fini() to block until all
    // hStreams activity completes, to tear down the initialization.
    hStreams_AtomicCounter         callCounter;
    // The hStreamsInitLock ensures that hStreams_Init is executed by only one thread
    // of execution at a time.  See hStreams_Init implementation for more information.
    hStreams_Lock                  hStreamsInitLock;
    // The hStreamsFiniLock ensures that hStreams_Fini is executed by only one thread
    // of execution at a time.  See hStreams_Fini implementation for more information.
    hStreams_Lock                  hStreamsFiniLock;
    /// Items that are associated with physical domains (cards)
    uint32_t                       myNumPhysDomains;      // 0 or more cards in the system
    uint32_t                       myActivePhysDomains;   // 0 or more active cards in the system,
    //  as reflected by COIEngineGetHandle
    bool                           homogeneous;           // true if all domains have the same resources
    HSTR_COIPROCESS                     dummyCOIProcess;       // Valid/dummy for COIBufferCreateFromMemory

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4324 ) /* Disable warning for: 'lastError' : structure was padded due to __declspec(align()) */
#endif
    HSTR_ALIGN(64)
    volatile long                  lastError;             // last hStreams error recorded
    HSTR_ALIGN(64)
    HSTR_COIBUFFER                    dummy_buf;
    HSTR_ALIGN(64)
    uint64_t                     dummy_data;
#ifdef _WIN32
#pragma warning( pop )
#endif
} hStreamHostProcess;

// The ConvertToUint64_t template is used to allow data to safely get passed the calling conventions
// in the hstreams API by marshalling/demarshalling data from/to source/sink execution environments.
// For an example of its use, see ref_code/common/hStreams_custom.cpp and
// ref_code/common/hStreams_custom_kernels_sink.cpp
template<class TYPE, const size_t sz>
class ConvertToUint64_t
{
public:

    // We abort compilation if the expression is not true, which is that the size of the TYPE parameter is less than
    // or equal to the sz parameter timessizeof(uint64_t).
    HSTR_STATIC_ASSERT(sizeof(TYPE) <= sz *sizeof(uint64_t), sizeof_TYPE_must_be_less_than_or_equal_to_sz_times_sizeof_uint64_t);

    typedef uint64_t conversion_t[sz];

    ConvertToUint64_t()
    {
        memset(this, 0, sizeof(*this));
    }

    void Set(const TYPE &t)
    {
        mUnion.t = t;
    }

    void Set_uint64_t(uint64_t u64, size_t idx = 0)
    {
        mUnion.c[idx] = u64;
    }

    TYPE &Get(void)
    {
        return mUnion.t;
    }

    uint64_t Get_uint64_t(size_t idx = 0)
    {
        return mUnion.c[idx];
    }

private:
    union {
        TYPE         t;
        conversion_t c;
    } mUnion;
};

typedef ConvertToUint64_t<char, 1>           charToUint64_t;
typedef ConvertToUint64_t<float, 1>          floatToUint64_t;
typedef ConvertToUint64_t<double, 1>         doubleToUint64_t;
typedef ConvertToUint64_t<MKL_Complex8, 1>   MKL_Complex8ToUint64_t;
typedef ConvertToUint64_t<MKL_Complex16, 2>  MKL_Complex16ToUint64_t;

#ifndef _WIN32
#define HSTR_TYPEOF typeof
#else
#define HSTR_TYPEOF decltype
#endif

///////////////////////////////////////////////////////////////////
///
/// Fetch executable name that is currently running on the source.  Trims the directory path and
///  '.exe' file extension from windows.
///
/// @param  out_buff
///         [out] The buffer that will receive the name of the executable.
///
/// @return HSTR_RESULT_SUCCESS if we successfully fetched the executable
///         name, and we copied the name to out_buff.
///
/// @return HSTR_RESULT_BUFF_TOO_SMALL if out_buff was too small to receive
///         the entire name.
///
/// @return HSTR_RESULT_INTERNAL_ERROR when an internal error occurs.
///
HSTR_RESULT
hStreams_FetchExecutableName(std::string &out_buff);

///////////////////////////////////////////////////////////////////////////////////////
///
//  hStreams_LoadSinkSideLibrariesMIC
/// @brief  Load the sink side libraries into the coi process running on the MIC sink.
///
/// @param  in_COIProcess
///         [in] The HSTR_COIPROCESS where to load the libraries.
///
/// @param  in_ExecutableFileName
///         [in] The name of the file name, less dir path, and file extension.
///
/// @return HSTR_RESULT_SUCCESS if we successfully loaded all MIC libraries.
///
/// @return HSTR_RESULT_BAD_NAME if a library is explicitly identified
///         in HSTR_OPTIONS and we can't find it in the path specified by environment
///         variable mic_sink_ld_library_path_env_name.
///////////////////////////////////////////////////////////////////////////////////////
HSTR_RESULT
hStreams_LoadSinkSideLibrariesMIC(HSTR_COIPROCESS coi_process, std::vector<HSTR_COILIBRARY> &out_loadedLibs, const std::string &in_ExecutableFileName);

///////////////////////////////////////////////////////////////////////////////////////
///
//  hStreams_LoadSinkSideLibrariesHost
/// @brief  Load the host side libraries into the host side sink worker on host.
///
/// @param  in_ExecutableFileName
///         [in] The name of the file name, less dir path, and file extension.
///
/// @return HSTR_RESULT_SUCCESS if we successfully loaded all host libraries.
///
/// @return HSTR_RESULT_BAD_NAME if a library is explicitly identified
///         in HSTR_OPTIONS and we can't find it in the path specified by environment
///         variable host_sink_ld_library_path_env_name.
///////////////////////////////////////////////////////////////////////////////////////
HSTR_RESULT
hStreams_LoadSinkSideLibrariesHost(std::string const &in_ExecutableFileName, std::vector<LIB_HANDLER::handle_t> &loaded_libs_handles);

///////////////////////////////////////////////////////////////////////////////////////
///
//  hSteaams_SleepMS
/// @brief Sleep for specified number of milliseconds.
///
/// @param  ms How long to sleep in milliseconds.
///
///////////////////////////////////////////////////////////////////////////////////////
inline void hStreams_SleepMS(unsigned int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/////////////////////////////////////////////////////////
///
// findFileName
/// @brief Find a filename in the SINK_LD_LIBRARY_PATH or
/// HOST_SINK_LD_LIBRARY_PATH (for host loading).
///
/// @param fileName path to the file
///
/// @param env_variable_path env variable in which path to libraries are kept
///
/// @return If the return value is empty, that means the conditions were
/// such that it could not find the file.
///
/////////////////////////////////////////////////////////
std::string findFileName(const char *fileName, const char *env_variable_path);

/////////////////////////////////////////////////////////
///
/// @brief Get next available logical domain ID
///
/////////////////////////////////////////////////////////
HSTR_LOG_DOM getNextLogDomID();

/////////////////////////////////////////////////////////
///
// HSTR_RETURN
/// @brief Saving passed __expr (HSTR_RESULT) to lastError if different
///         then HSTR_RESULT_SUCCESS and returning this value as well.
///
/// @param __expr HSTR_RESULT which should saved to lastError
///         if necessary, you might pass here call to function too.
///
/// @return returning passed untouched __expr
///
/////////////////////////////////////////////////////////
#define HSTR_RETURN(__expr)                                             \
    DISABLE_4127_WARNING                                                \
    do                                                                  \
    {                                                                   \
        HSTR_RESULT __hstream_result = __expr;                          \
        if (__hstream_result != HSTR_RESULT_SUCCESS)                    \
        {                                                               \
            hStreams_AtomicStore(hstr_proc.lastError, __hstream_result);\
        }                                                               \
        return __hstream_result;                                        \
    }                                                                   \
    while (0)                                                           \
        ENABLE_4127_WARNING

#endif
/// @}
