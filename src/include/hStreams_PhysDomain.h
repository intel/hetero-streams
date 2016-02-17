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

#ifndef HSTREAMS_PHYSDOMAIN_H
#define HSTREAMS_PHYSDOMAIN_H

#include "hStreams_common.h"
#include "hStreams_types.h"
#include "hStreams_locks.h"
#include "hStreams_helpers_source.h"
#include "hStreams_COIWrapper.h"

#include <vector>
#include <map>
#include <string>

// forward declarations
class hStreams_LogDomain;
class hStreams_PhysStream;

/// @brief Common interface class for various physical domain implementations
class hStreams_PhysDomain
{
    /// @brief ID of the physical domain
    const HSTR_PHYS_DOM id_;
    /// @brief Maximum allowed mask of logical CPUs the user can create
    ///     logical domains and streams within
    const hStreams_CPUMask max_cpu_mask_;
    /// @brief A CPU mask in which we _suggest_ that the user doesn't
    ///     create streams for performance-sensitive applications
    const hStreams_CPUMask avoid_cpu_mask_;
    /// @brief Basically a count of the max cpu mask
    const uint32_t num_threads_;

    typedef std::vector<hStreams_LogDomain *> LogDomainsContainer;
    /// @brief "Links" to all the logical domains in this physical domain
    LogDomainsContainer log_domains_;
    typedef std::map<std::string, uint64_t> SinkFuncAddressesContainer;
    /// @brief A cache of sink-side function addresses, to avoid constant lookups
    ///     which might be costly
    SinkFuncAddressesContainer sink_functions_addresses_;
    /// @brief A read-write mutex to internally synchronize access to the sink-side
    ///     functions cache
    hStreams_RW_Lock sink_functions_addresses_lock_;
    /// @brief An incrementally maintained oversubscription array.
    ///
    /// Each entry in this vector is a count of how many physical streams overlap on
    /// that logical CPU.
    std::vector<uint32_t> oversubscription_array_;

public:
    /// @brief Get a copy of the max cpu mask
    hStreams_CPUMask getMaxCPUMask() const;
    /// @brief Get a copy of the avoid cpu mask
    hStreams_CPUMask getAvoidCPUMask() const;
    /// @brief Get the count of all logical CPUs in this physical domain
    uint32_t getNumThreads() const;

    // Most of these attributes can probably be made private with getter methods.
    // we might want to look into that further down the road.

    /// @brief Instruction Set Architecture of the physical domain
    const HSTR_ISA_TYPE isa;
    /// @brief The maximum frequency of a core
    const uint32_t core_max_freq_MHz;
    /// @brief The amount of available memory on that domain, in bytes.
    const uint64_t available_memory;

    virtual ~hStreams_PhysDomain();

    /// Report mask _not_ occupied by any stream in any logical domain contained
    /// in this physical domain and valid for use.
    ///
    /// @note If a logical domain exists with no streams create in it, the logical
    ///     CPUs will continue to show up in the available cpu mask. It is the
    ///     streams which make the CPUs occupied.
    hStreams_CPUMask getAvailableStreamCPUMask() const;

    /// @brief Copy the oversubscription array into the argument.
    /// @param[out] array The memory to write the oversubscription array to. Must
    ///     contain at least getNumThreads() entries.
    ///
    /// @note Size of array must be previously validated against getNumThreads().
    ///     Otherwise, behaviour is undefined
    ///
    /// @sa hStreams_GetOversubscriptionLevel()
    void getOversubscriptionLevel(uint32_t *array) const;

    HSTR_PHYS_DOM id() const
    {
        return id_;
    }

    HSTR_COIPROCESS getCOIProcess() const
    {
        return impl_getCOIProcess();
    }

    /// @brief Spawn a new physical stream inside that physical domain.
    ///
    /// The caller is responsible for the cpu mask to be within the _logical_
    /// domain boundaries they wish the stream to respect.
    ///
    /// Implementation of this method _may_ check whether this cpu
    /// mask is valid with respect to the _physical_ domain.
    hStreams_PhysStream *createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask);

    /// @brief Internally cached function address lookup functionality
    /// @param[in] func_name The name of the function to be looked up
    /// @return Sink-side address of the function, 0 if not found or an error occured
    uint64_t fetchSinkFunctionAddress(std::string const &func_name);

    /// @brief Get a logical domain in this physical domain which would match the cpu mask.
    ///
    /// If found, out_overlap will be set to EXACT_OVERLAP.
    /// If not found, NULL pointer will be retuned and out_overlap will be set to either
    /// NO_OVERLAP or PARTIAL_OVERLAP to indicate whether the supplied CPU mask overlaps
    /// CPU mask of any logical domain in this physical domain
    ///
    /// out_overlap is optional, i.e. can be NULL
    hStreams_LogDomain *lookupLogDomainByCPUMask(hStreams_CPUMask const &mask, HSTR_OVERLAP_TYPE *out_overlap) const;

    /// @brief A hook for notifying the physical domain that a new logical domain has been created in it
    void addLogDomainMapping(hStreams_LogDomain &log_dom);
    /// @brief A hook for notifying the physical domain that a logical domain has been deleted from it
    void delLogDomainMapping(hStreams_LogDomain &log_dom);
    /// @brief Get the number of logical domains present in this physical domain
    /// @sa hStreams_GetNumLogDomains()
    uint32_t getNumLogDomains();
    /// @brief Write out the IDS of logical domains in this physical domain
    /// @sa hStreams_GetLogDomainIDList()
    void getLogDomainIDs(HSTR_LOG_DOM *ptr, uint32_t num, uint32_t *num_written, uint32_t *num_present) const;
    /// @brief A hook for notifyin the physical domain that a physical stream has been deleted from it
    ///
    /// Used for incrementally maintaining the oversubscription level array
    void processPhysStreamDestroy(hStreams_PhysStream const &phys_stream);
protected:
    hStreams_PhysDomain(
        HSTR_PHYS_DOM id,
        HSTR_ISA_TYPE isa,
        uint32_t core_max_freq_MHz,
        uint64_t available_memory,
        hStreams_CPUMask const &max_cpu_mask,
        hStreams_CPUMask const &avoid_cpu_mask
    );

private:
    // assignment operator is prohibited
    hStreams_PhysDomain &operator=(const hStreams_PhysDomain &other);
    // virtual functions don't care about access modifiers
    virtual HSTR_COIPROCESS impl_getCOIProcess() const = 0;
    virtual hStreams_PhysStream *impl_createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask) = 0;

    /// @brief Look up the internal cache of function sink-side addresses
    /// @return Function's sink-side address, 0 if the function is not present in the cache.
    /// @sa hStreams_PhysDomain::setSinkAddress()
    /// @note This function is internally synchronized
    uint64_t getSinkAddress(std::string const &func_name);
    /// @brief Set the function's sink-side address for the internal cache
    /// @sa hStreams_PhysDomain::getSinkAddress()
    /// @note This function is internally synchronized
    void setSinkAddress(std::string const &func_name, uint64_t sink_address);
    /// @brief Underlying implementation of looking up a function's address
    /// @note Implementations of hStreams_PhysDomain must provide it.
    /// @note No cachin in the implementation should happen. The cache is maintained
    ///     within the hStreams_PhysDomain class
    virtual uint64_t impl_fetchSinkFunctionAddress(std::string const &func_name) = 0;

    /// @brief Manipulate the oversubscription array
    ///
    /// Adds \c v to the entry in the oversubscription array if a bit in the \c mask is set.
    /// @note v can be negative
    void modifyOversubscriptionArray(hStreams_CPUMask const &mask, int32_t v);
};

#endif /* HSTREAMS_PHYSDOMAIN_H */
