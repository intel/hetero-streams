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

#ifndef HSTREAMS_HELPERS_SOURCE_H
#define HSTREAMS_HELPERS_SOURCE_H

#include "hStreams_types.h"
#include "hStreams_common.h"
#include "hStreams_source.h"
#include "hStreams_internal.h"
#include "hStreams_helpers_common.h"

#include <limits>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifndef _WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif

#ifdef _WIN32
#undef max // Disable max() macro to use std::numeric_limits<T>::max()
#endif


// Returns true if a + b would overflow the underlying type
template <class T>
bool addition_overflow(T const &a, T const &b)
{
    HSTR_STATIC_ASSERT(!std::numeric_limits<T>::is_signed, not_implemented_for_signed_types);
    return std::numeric_limits<T>::max() - b < a;
}

// Returns true if a + b > c, with a safeguard against a+b overflowing
template <class T>
bool add_gt(T const &a, T const &b, T const &c)
{
    HSTR_STATIC_ASSERT(!std::numeric_limits<T>::is_signed, not_implemented_for_signed_types);
    return addition_overflow(a, b) || (a + b > c);
}

// Work around the limitations of using pure arrays types in C/C++
// (can't return them from functions, are always passed "by-reference" etc.)
// As a reminder, HSTR_CPU_MASK == uint64_t[16]
struct hStreams_CPUMask {
#ifndef _WIN32
    typedef cpu_set_t native_form_t;
#else
    typedef DWORD_PTR native_form_t;
#endif
    HSTR_CPU_MASK mask;

    hStreams_CPUMask();
    hStreams_CPUMask(HSTR_CPU_MASK cpu_mask);

    void computeNativeForm(native_form_t &native_form) const;

    hStreams_CPUMask &operator=(hStreams_CPUMask const &other)
    {
        memcpy(mask, other.mask, sizeof(HSTR_CPU_MASK));
        return *this;
    }
};
bool operator==(hStreams_CPUMask const &m1, hStreams_CPUMask const &m2);
bool operator!=(hStreams_CPUMask const &m1, hStreams_CPUMask const &m2);

class hStreams_MemAlignedAllocator
{
public:
    static void *alloc(uint64_t len);
    static void dealloc(void *data_ptr);
};


class hStreams_PhysStream;
////////////////////////////////////////////////////////////////////
///
//  hStreams_helper_func_19parm
/// @brief  Call a sink-side helper function with 19 parameters.
///
/// @param  in_phStr
///         [in] pointer to the hStream that the RunFunction should be queued up in
///
/// @param  in_FunctionName
///         [in] name of the remote function to be invoked
///
/// @param  in_parameter[N]
///         [in] The 19 scalar parameters
///
////////////////////////////////////////////////////////////////////
HSTR_RESULT
hStreams_helper_func_19parm(
    hStreams_PhysStream &in_phStr,
    const char       *in_pFuncName,
    uint64_t          in_parameter0,
    uint64_t          in_parameter1,
    uint64_t          in_parameter2,
    uint64_t          in_parameter3,
    uint64_t          in_parameter4,
    uint64_t          in_parameter5,
    uint64_t          in_parameter6,
    uint64_t          in_parameter7,
    uint64_t          in_parameter8,
    uint64_t          in_parameter9,
    uint64_t          in_parameter10,
    uint64_t          in_parameter11,
    uint64_t          in_parameter12,
    uint64_t          in_parameter13,
    uint64_t          in_parameter14,
    uint64_t          in_parameter15,
    uint64_t          in_parameter16,
    uint64_t          in_parameter17,
    uint64_t          in_parameter18);

class StringBuilder
{
    std::stringstream ss_;
public:
    template <class T>
    StringBuilder &operator<<(T const &x)
    {
        ss_ << x;
        return *this;
    }
    operator std::string() const
    {
        return ss_.str();
    }
    static std::string hex(uint64_t number)
    {
        return StringBuilder() << "0x" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << number;
    }
};

// HSTRInitializer allows to initialize hStreams when it is not certain that
// library should be still initialized after leaving current scope.
//
// hStreams_Init is call in constructor.
// By default hStreams_Fini is called during destruction of HSTRInitializer's instance.
// In order to prevent hStreams_Fini being called, call the member function dontFini()
//
// Finalization will be done only if hStreams was not already initialized when
// object was created.


class HSTRInitializer
{
    bool should_fini_;
    const HSTR_RESULT init_result_;

public:
    HSTRInitializer(const char *interface_version) :
        // We should only Fini if the library hasn't been intialized beforehand
        should_fini_(hStreams_IsInitialized() != HSTR_RESULT_SUCCESS),
        init_result_(hStreams_InitInVersion(interface_version))
    {
    }

    ~HSTRInitializer()
    {
        if (should_fini_) {
            hStreams_Fini();
        }
    }

    HSTR_RESULT getInitResult()
    {
        return init_result_;
    }

    void dontFini()
    {
        should_fini_ = false;
    }
};

// The DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION() macro declares a thread-safe function
// for getting one member of the HSTR_OPTIONS struct.  These functions are defined in
// hStreams_common.cpp
#define DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(MEMBER_NAME, MEMBER_TYPE) \
    MEMBER_TYPE hStreams_GetOptions_ ## MEMBER_NAME (void);

// Declare each of the thread safe functions for getting the current value of
// the members of the hstreams options structure:
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(dep_policy, HSTR_DEP_POLICY)
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(phys_domains_limit, uint32_t)
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(openmp_policy, HSTR_OPENMP_POLICY)
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(time_out_ms_val, int)
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(_hStreams_FatalError, hStreams_FatalError_Prototype_Fptr)
DECLARE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(kmp_affinity, HSTR_KMP_AFFINITY)

namespace detail
{
/**
 * Helper function to return whether an argument is present in a container with
 * standard interfaces or not
 */
template <class C, class V>
inline bool in_container(const C &container, const V &value)
{
    if (std::find(container.begin(), container.end(), value) == container.end()) {
        return false;
    } else {
        return true;
    }
}

} // namespace detail


#endif /* HSTREAMS_HELPERS_SOURCE_H */

