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

#include "hStreams_PhysStreamHost.h"
#include "hStreams_Logger.h"

hStreams_PhysStreamHost::hStreams_PhysStreamHost(
    hStreams_LogDomain &log_dom,
    hStreams_CPUMask const &cpu_mask
)
    :
    hStreams_PhysStream(log_dom, cpu_mask),
    hostSinkWorker_(new hStreams_HostSideSinkWorker(cpu_mask))
{
}

hStreams_PhysStreamHost::~hStreams_PhysStreamHost()
{
}

HSTR_RESULT hStreams_PhysStreamHost::impl_enqueueFunction(
    std::vector<uint64_t> &args,
    std::vector<HSTR_EVENT> &input_deps,
    void *ret_val, uint16_t ret_val_size, HSTR_EVENT *ret_event
)
{
    HSTR_EVENT an_event;
    HSTR_COIRESULT result = hStreams_COIWrapper::COIEventRegisterUserEvent(&an_event);
    if (result != HSTR_COI_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                << "A problem encountered while registering event: "
                << hStreams_COIWrapper::COIResultGetName(result);

        return HSTR_RESULT_INTERNAL_ERROR;
    }
    *ret_event = an_event;
    std::unique_ptr<ComputePayload> payload(new ComputePayload(args, input_deps,
                                            ret_val, ret_val_size, an_event));

    std::unique_ptr<Action> new_action(new Action(COMPUTE, std::move(payload)));

    return hostSinkWorker_->putAction(std::move(new_action));
}

