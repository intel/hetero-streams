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
#ifndef HSTREAMS_PHYSDOMAINCOI_H
#define HSTREAMS_PHYSDOMAINCOI_H

#include <vector>

#include "hStreams_PhysDomain.h"
#include "hStreams_COIWrapper.h"

class hStreams_LogDomain;

/// @brief An implementation of a physical domain which represents the physical
///     domain on a local, PCIe-accessed x100 card.
class hStreams_PhysDomainCOI : public hStreams_PhysDomain
{
    /// @brief A pre-created handle to the COI process
    ///
    /// This handle has to be pre-created before the physical domain is instantiated. It
    /// is obtained through COIProcessCreate* calls and passed as an argument to
    /// hstreams_PhysDomainCOI constructor.
    const HSTR_COIPROCESS coi_process_;
    /// @brief A pre-obtained handle to the thunk function
    ///
    /// This handle has to be pre-created before the physical domain is instantiated. It
    /// is obtained through COIProcessGetFunctionHandles and passed as an argument to
    /// hstreams_PhysDomainCOI constructor.
    const HSTR_COIFUNCTION thunk_func_;
    /// @brief A pre-obtained handle to function which can perform sink-side dynamic symbol lookup address
    ///
    /// This handle has to be pre-created before the physical domain is instantiated. It
    /// is obtained through COIProcessGetFunctionHandles and passed as an argument to
    /// hstreams_PhysDomainCOI constructor.
    const HSTR_COIFUNCTION fetch_addr_func_;
    /// @brief pre-created helper pipeline used for sink-side function address lookups
    ///
    /// This pipeline has to be pre-created before the physical domain is instantiated.
    /// The handle is supplied as an argument to the constructor of the hstreams_PhysDomainCOI
    const HSTR_COIPIPELINE helper_pipeline_;
    typedef std::vector<HSTR_COILIBRARY> SinkLibsContainer;
    /// @brief Libraries loaded to the sink process on user's request.
    SinkLibsContainer sink_libs_;
public:
    /// @param[in] id The externally-visible ID of the physical domain
    /// @param[in[ coi_process A handle to a pre-created COI process
    /// @param[in] coi_eng_info Information about the domain obtained from COI
    /// @param[in] sink_libs A vector of handles to libraries already loaded to the process
    /// @param[in] thunk_func A pre-looked-up COI handle to the sink-side thunk instance
    /// @param[in] fetch_addr_func A pre-looked-up COI handle to the sink-side instance of
    ///     function which looks up other functions' addresses
    /// @param[in] helper_pipeline A handle to a pre-created pipeline which will
    ///     be used for sink-side function address lookups
    ///
    /// @note The separate, helper pipeline is necessary because it is required that
    ///     enqueuing operations to a stream be asynchronous. At the same time, it may be
    ///     necessary to look up the sink-side function's address prior to the actual enqueuing.
    ///     If that lookup results in a cache miss and has to go to be performed on the sink,
    ///     if the stream's pipeline were to be used, the lookup operation would block until
    ///     ALL the actions previously enqueued in the stream have completed their execution
    ///     (due to how COI's pipeline scheduler works). It is therefore necessary to use a
    ///     second, unrelated pipeline for those lookups. That helper pipeline can be shared
    ///     across all physical streams in a given physical domain since all streams in the same
    ///     physical domain belong to the same process (even if there are multiple logical domains
    ///     within that physical domain).
    hStreams_PhysDomainCOI(
        HSTR_PHYS_DOM id,
        HSTR_COIPROCESS coi_process,
        HSTR_COI_ENGINE_INFO const &coi_eng_info,
        std::vector<HSTR_COILIBRARY> const &sink_libs,
        HSTR_COIFUNCTION thunk_func,
        HSTR_COIFUNCTION fetch_addr_func,
        HSTR_COIPIPELINE helper_pipeline
    );

    /// will unload the libraries and destroy the COI process
    ~hStreams_PhysDomainCOI();

private:
    HSTR_COIPROCESS impl_getCOIProcess() const
    {
        return coi_process_;
    }
    hStreams_PhysStream *impl_createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask);
    /// @brief Looks up a function's address on a COI-served physical domain
    uint64_t impl_fetchSinkFunctionAddress(std::string const &func_name);
};

#endif /* HSTREAMS_PHYSDOMAINCOI_H */
