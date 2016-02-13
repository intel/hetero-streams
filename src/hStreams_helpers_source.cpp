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

#include "hStreams_helpers_source.h"

#include "hStreams_internal_vars_source.h"
#include "hStreams_PhysBuffer.h"
#include "hStreams_PhysDomain.h"
#include "hStreams_PhysStream.h"
#include "hStreams_exceptions.h"

hStreams_CPUMask::hStreams_CPUMask()
{
    HSTR_CPU_MASK_ZERO(mask);
}

hStreams_CPUMask::hStreams_CPUMask(HSTR_CPU_MASK cpu_mask)
{
    memcpy(mask, cpu_mask, sizeof(HSTR_CPU_MASK));
}

#ifndef _WIN32
void hStreams_CPUMask::computeNativeForm(cpu_set_t &cpu_set) const
{
    CPU_ZERO(&cpu_set);
    for (int i = 0; i < 16 * 64; ++i) {
        if (HSTR_CPU_MASK_ISSET(i, mask)) {
            CPU_SET(i, &cpu_set);
        }
    }
}
#else
void hStreams_CPUMask::computeNativeForm(DWORD_PTR &cpu_set) const
{
    cpu_set = 0;
    for (int i = 0; i < 64; ++i) {
        if (HSTR_CPU_MASK_ISSET(i, mask)) {
            cpu_set |= (1ll) << i;
        }
    }
}
#endif

bool operator==(hStreams_CPUMask const &m1, hStreams_CPUMask const &m2)
{
    return HSTR_CPU_MASK_EQUAL(m1.mask, m2.mask) != 0;
}

bool operator!=(hStreams_CPUMask const &m1, hStreams_CPUMask const &m2)
{
    return !(m1 == m2);
}

void *hStreams_MemAlignedAllocator::alloc(uint64_t len)
{
    //TODO: Align to 64 or maybe something else ?
#ifndef _WIN32
    void *mem;
    int ret = posix_memalign(&mem, 64, len);

    return (ret) ? NULL : mem;
#else
    return _aligned_malloc(len, 64);
#endif
}

void hStreams_MemAlignedAllocator::dealloc(void *data_ptr)
{
#ifndef _WIN32
    free(data_ptr);
#else
    _aligned_free(data_ptr);
#endif
}

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
    uint64_t          in_parameter18)
{
    HSTR_EVENT completion_event;
    std::vector<uint64_t> scalar_args;
    std::vector<hStreams_PhysBuffer *> buffer_args;
    std::vector<uint64_t> buffer_offsets;

    scalar_args.reserve(19);
    scalar_args.push_back(in_parameter0);
    scalar_args.push_back(in_parameter1);
    scalar_args.push_back(in_parameter2);
    scalar_args.push_back(in_parameter3);
    scalar_args.push_back(in_parameter4);
    scalar_args.push_back(in_parameter5);
    scalar_args.push_back(in_parameter6);
    scalar_args.push_back(in_parameter7);
    scalar_args.push_back(in_parameter8);
    scalar_args.push_back(in_parameter9);
    scalar_args.push_back(in_parameter10);
    scalar_args.push_back(in_parameter11);
    scalar_args.push_back(in_parameter12);
    scalar_args.push_back(in_parameter13);
    scalar_args.push_back(in_parameter14);
    scalar_args.push_back(in_parameter15);
    scalar_args.push_back(in_parameter16);
    scalar_args.push_back(in_parameter17);
    scalar_args.push_back(in_parameter18);

    CHECK_HSTR_RESULT(
        in_phStr.enqueueFunction(in_pFuncName, scalar_args, buffer_args, buffer_offsets,
                                 NULL, 0, &completion_event)
    );

    // No waits on the data, so this can be truly async
    return HSTR_RESULT_SUCCESS;
}

// The DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION() macro defines a thread-safe function
// for getting one member of the HSTR_OPTIONS struct.  These functions are declared in
// hStreams_internal.h
#define DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(MEMBER_NAME)                                       \
    HSTR_TYPEOF(((HSTR_OPTIONS*)0)->MEMBER_NAME) hStreams_GetOptions_ ## MEMBER_NAME (void) \
    {                                                                                              \
        try {                                                                                      \
            hStreams_RW_Scope_Locker_Unlocker hstreams_options_rw_lock(globals::options_lock,      \
                    hStreams_RW_Lock::HSTR_RW_LOCK_READ);                                          \
            return globals::options . MEMBER_NAME;                                                 \
        } catch (...) {                                                                            \
            hStreams_handle_exception();                                                           \
            /* Unlocked return fallback */                                                         \
            return globals::options . MEMBER_NAME;                                                  \
        }                                                                                          \
    }

// Define each of the thread safe functions for getting the current value of
// the (non-deprecated) members of the hstreams options structure:
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(dep_policy)
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(phys_domains_limit)
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(openmp_policy)
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(time_out_ms_val)
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(_hStreams_FatalError)
DEFINE_GET_HSTR_OPTIONS_MEMBER_FUNCTION(kmp_affinity)

