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

#include "hStreams_PhysDomain.h"
#include "hStreams_LogDomain.h"
#include "hStreams_PhysStream.h"
#include "hStreams_internal_vars_common.h"

hStreams_PhysDomain::hStreams_PhysDomain(
    HSTR_PHYS_DOM my_id,
    HSTR_ISA_TYPE my_isa,
    uint32_t my_core_max_freq_MHz,
    uint64_t my_available_memory,
    hStreams_CPUMask const &max_cpu_mask,
    hStreams_CPUMask const &avoid_cpu_mask
)
    :
    id_(my_id),
    isa(my_isa),
    core_max_freq_MHz(my_core_max_freq_MHz),
    available_memory(my_available_memory),
    max_cpu_mask_(max_cpu_mask),
    avoid_cpu_mask_(avoid_cpu_mask),
    oversubscription_array_(HSTR_CPU_MASK_COUNT(max_cpu_mask.mask), 0),
    num_threads_(HSTR_CPU_MASK_COUNT(max_cpu_mask.mask))
{
}

hStreams_PhysDomain::~hStreams_PhysDomain()
{
}

hStreams_CPUMask hStreams_PhysDomain::getMaxCPUMask() const
{
    return max_cpu_mask_;
}

hStreams_CPUMask hStreams_PhysDomain::getAvoidCPUMask() const
{
    return avoid_cpu_mask_;
}


hStreams_LogDomain *hStreams_PhysDomain::lookupLogDomainByCPUMask(hStreams_CPUMask const &mask, HSTR_OVERLAP_TYPE *out_overlap) const
{
    HSTR_CPU_MASK tmp_mask;
    for (LogDomainsContainer::const_iterator it = log_domains_.begin(); it != log_domains_.end(); ++it) {
        // check for full overlap
        if (mask == (*it)->getCPUMask()) {
            if (out_overlap) {
                *out_overlap = HSTR_EXACT_OVERLAP;
            }
            return *it;
        }
        // check for partial overlap. If it partially overlaps, we know we
        // won't find a logical domain with the input CPU mask as the logical domains
        // must be disjoint
        HSTR_CPU_MASK_AND(tmp_mask, mask.mask, (*it)->getCPUMask().mask);
        if (HSTR_CPU_MASK_COUNT(tmp_mask)) {
            if (out_overlap) {
                *out_overlap = HSTR_PARTIAL_OVERLAP;
            }
            return NULL;
        }
    }
    if (out_overlap) {
        *out_overlap = HSTR_NO_OVERLAP;
    }
    return NULL;
}

void hStreams_PhysDomain::addLogDomainMapping(hStreams_LogDomain &log_dom)
{
    log_domains_.push_back(&log_dom);
}

void hStreams_PhysDomain::delLogDomainMapping(hStreams_LogDomain &log_dom)
{
    for (LogDomainsContainer::iterator it = log_domains_.begin(); it != log_domains_.end(); ++it) {
        if ((*it)->id() == log_dom.id()) {
            log_domains_.erase(it);
            return;
        }
    }
    // TODO log the failure? Report it in DEBUG version of the library?
}

uint32_t hStreams_PhysDomain::getNumLogDomains()
{
    return (uint32_t) log_domains_.size();
}

void hStreams_PhysDomain::getLogDomainIDs(HSTR_LOG_DOM *ptr, uint32_t num, uint32_t *num_written, uint32_t *num_present) const
{
    uint32_t index = 0;
    for (LogDomainsContainer::const_iterator it = log_domains_.begin(); it != log_domains_.end(); ++it, ++index) {
        //Don't fill after array ends
        if (index == num) {
            break;
        }

        ptr[index] = (*it)->id();
    }

    *num_written = index;
    *num_present = (uint32_t) log_domains_.size();
}

uint64_t hStreams_PhysDomain::getSinkAddress(std::string const &func_name)
{
    hStreams_RW_Scope_Locker_Unlocker cache_lock(sink_functions_addresses_lock_,
            hStreams_RW_Lock::HSTR_RW_LOCK_READ);
    SinkFuncAddressesContainer::const_iterator it = sink_functions_addresses_.find(func_name);
    if (it == sink_functions_addresses_.end()) {
        return 0;
    }
    return it->second;
}

void hStreams_PhysDomain::setSinkAddress(std::string const &func_name, uint64_t sink_addr)
{
    hStreams_RW_Scope_Locker_Unlocker cache_lock(sink_functions_addresses_lock_,
            hStreams_RW_Lock::HSTR_RW_LOCK_WRITE);
    sink_functions_addresses_[func_name] = sink_addr;
}

uint64_t hStreams_PhysDomain::fetchSinkFunctionAddress(std::string const &func_name)
{
    uint64_t ret = getSinkAddress(func_name);
    if (0 != ret) {
        return ret;
    }
    // Return NULL for hStreams functions using MKL when is disabled
    if (globals::mkl_interface == HSTR_MKL_NONE) {
        if (
            (func_name == "hStreams_sgemm_sink") ||
            (func_name == "hStreams_dgemm_sink") ||
            (func_name == "hStreams_cgemm_sink") ||
            (func_name == "hStreams_zgemm_sink")) {
            return 0;
        }
    }
    // NOTE this will repeat unsuccessful lookups
    ret = impl_fetchSinkFunctionAddress(func_name);
    if (0 != ret) {
        setSinkAddress(func_name, ret);
    }
    return ret;
}

hStreams_PhysStream *hStreams_PhysDomain::createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask)
{
    hStreams_PhysStream *new_stream = impl_createNewPhysStream(log_dom, cpu_mask);
    if (NULL != new_stream) {
        modifyOversubscriptionArray(cpu_mask, 1);
    }
    return new_stream;
}

hStreams_CPUMask hStreams_PhysDomain::getAvailableStreamCPUMask() const
{
    hStreams_CPUMask ret;
    for (int i = 0; i < oversubscription_array_.size(); ++i) {
        if (0 == oversubscription_array_[i]) {
            HSTR_CPU_MASK_SET(i, ret.mask);
        }
    }
    return ret;
}

void hStreams_PhysDomain::processPhysStreamDestroy(hStreams_PhysStream const &phys_stream)
{
    modifyOversubscriptionArray(phys_stream.getCPUMask(), -1);
}

void hStreams_PhysDomain::modifyOversubscriptionArray(hStreams_CPUMask const &mask, int32_t v)
{
    for (unsigned int j = 0; j < num_threads_; ++j) {
        if (HSTR_CPU_MASK_ISSET(j, mask.mask)) {
            oversubscription_array_[j] += v;
        }
    }
}

void hStreams_PhysDomain::getOversubscriptionLevel(uint32_t *array) const
{
    for (int i = 0; i < oversubscription_array_.size(); ++i) {
        *(array + i) = oversubscription_array_[i];
    }
}

uint32_t hStreams_PhysDomain::getNumThreads() const
{
    return num_threads_;
}
