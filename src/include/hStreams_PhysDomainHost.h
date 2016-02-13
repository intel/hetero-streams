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

#ifndef HSTREAMS_PHYSDOMAINHOST_H
#define HSTREAMS_PHYSDOMAINHOST_H

#include "hStreams_PhysDomain.h"
#include "hStreams_PhysStreamHost.h"
#include "hStreams_internal_types_common.h"
/// @brief An implementation of a physical domain which represents the physical
///     domain the "source" is running on, i.e. localhost.
class hStreams_PhysDomainHost : public hStreams_PhysDomain
{
    /// @brief Some valid COI process, not \c HSTR_COI_PROCESS_SOURCE
    /// @note We need this as the \c COIBufferCreate*() family needs a valid COI process
    ///     handle to do the job, i.e. it is not possible to instantiate a COI buffer with
    ///     the special value \c HSTR_COI_PROCESS_SOURCE as the process it should be valid in
    /// @note This is the primary limitation for having hStreams work without COI, e.g.
    ///     for future host-only offloads. You'll still need a valid HSTR_COIPROCESS handle
    ///     to some remote process (e.g. a local MIC).
    HSTR_COIPROCESS coi_proc_;

    /// @brief List of handles of loaded libraries
    const std::vector<LIB_HANDLER::handle_t> loaded_libs_handles_;
public:
    /// @param[in] coi_proc some valid coi process handle, specifically NOT
    ///     HSTR_COI_PROCESS_SOURCE (see \c coi_proc_).
    /// @param[in] loaded_libs_handles list of handles of loaded libraries
    hStreams_PhysDomainHost(HSTR_COIPROCESS coi_proc, const std::vector<LIB_HANDLER::handle_t> &loaded_libs_handles);
    /// @brief Destructor, unloading libs that were previously loaded for the host
    virtual ~hStreams_PhysDomainHost();
private:
    /// @brief Returns the COI process handle the domain has been instantiated with
    HSTR_COIPROCESS impl_getCOIProcess() const;
    /// @brief Spawn a new physical stream.
    /// @return NULL for now as host-side streams are not implemented yet.
    hStreams_PhysStream *impl_createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask);
    /// @brief Looks up a function's address on the host
    uint64_t impl_fetchSinkFunctionAddress(std::string const &func_name);
};

#endif /* HSTREAMS_PHYSDOMAINHOST_H */
