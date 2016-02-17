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

#include <memory>
#include <functional>
#include "hStreams_internal.h"
#include "hStreams_helpers_source.h"
#include "hStreams_exceptions.h"
#include "hStreams_HostSideSinkWorker.h"
#include "hStreams_sink.h"
#include "hStreams_Logger.h"
#include "hStreams_COIWrapper.h"



// Declarations of functions from hStreams_sink.cpp
HSTREAMS_EXPORT
void hStreamsThunk(
    uint32_t         in_BufferCount,
    void           **in_ppBufferPointers,
    uint64_t        *in_pBufferLengths,
    void            *in_pMiscData,
    uint16_t         in_MiscDataLength,
    void            *in_pReturnValue,
    uint16_t         in_ReturnValueLength);

HSTREAMS_EXPORT
void hStreams_init_partition(
    uint64_t domain,
    uint64_t stream,
    uint64_t num_cores,
    uint64_t cpumask0,
    uint64_t cpumask1,
    uint64_t cpumask2,
    uint64_t cpumask3,
    uint64_t cpumask4,
    uint64_t cpumask5,
    uint64_t cpumask6,
    uint64_t cpumask7,
    uint64_t cpumask8,
    uint64_t cpumask9,
    uint64_t cpumask10,
    uint64_t cpumask11,
    uint64_t cpumask12,
    uint64_t cpumask13,
    uint64_t cpumask14,
    uint64_t cpumask15);

ComputePayload::ComputePayload(std::vector<uint64_t> &args,
                               std::vector<HSTR_EVENT> &input_deps,
                               void *ret_val, uint16_t &ret_val_size,
                               HSTR_EVENT ret_event) :
    args_(args), input_deps_(input_deps),
    ret_val_(ret_val), ret_val_size_(ret_val_size),
    ret_event_(ret_event)
{
}

Action::Action(ACTION_TYPE action_type, std::unique_ptr<ComputePayload> payload) :
    action_type_(action_type), payload_(std::move(payload))
{
}

Action::~Action()
{
}

std::unique_ptr<ComputePayload> &Action::getComputePayload()
{
    return payload_;
}

ACTION_TYPE Action::getActionType()
{
    return action_type_;
}

hStreams_SPSCQueue::hStreams_SPSCQueue()
{
}

hStreams_SPSCQueue::~hStreams_SPSCQueue()
{
}

void hStreams_SPSCQueue::add(std::unique_ptr<Action> action)
{
    hStreams_Scope_Locker_Unlocker autolock(mutex_);
    queue_.push(std::move(action));
    cond_var_.signal();
}

std::unique_ptr<Action> hStreams_SPSCQueue::popFront()
{
    hStreams_Scope_Locker_Unlocker autolock(mutex_);
    cond_var_.wait(mutex_, std::bind(&queue_t::empty, &queue_));
    //Else just grab action element
    std::unique_ptr<Action> action = std::move(queue_.front());
    queue_.pop();
    return std::move(action);
}

hStreams_HostSideSinkWorker::hStreams_HostSideSinkWorker(hStreams_CPUMask const &cpu_mask) :
    queue_(new hStreams_SPSCQueue()),
    thread_(new hStreams_Thread(&hStreams_HostSideSinkWorker::workerMainLoop, this)),
    cpu_mask_(cpu_mask),
    worker_status_(HSTR_RESULT_SUCCESS)
{

}

hStreams_HostSideSinkWorker::~hStreams_HostSideSinkWorker()
{
    // This is destructor, we need to handle the exception gracefully
    try {
        std::unique_ptr<ComputePayload> payload(nullptr);
        std::unique_ptr<Action> stop_action(new Action(STOP, std::move(payload)));

        HSTR_RESULT result = putAction(std::move(stop_action));
        if (result != HSTR_RESULT_SUCCESS) {
            HSTR_WARN(HSTR_INFO_TYPE_MISC) << "Could not signal host worker's thread to stop";
        }

        thread_->join();
    } catch (...) {
        hStreams_handle_exception();
    }
}

HSTR_RESULT hStreams_HostSideSinkWorker::putAction(std::unique_ptr<Action> action)
{
    if (worker_status_ != HSTR_RESULT_SUCCESS) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "Could not put an action on the host worker's queue. Worker status: " << worker_status_;

        return worker_status_;
    }
    queue_->add(std::move(action));
    return HSTR_RESULT_SUCCESS;
}

namespace
{
void set_affinity(hStreams_CPUMask const &cpu_mask)
{
#ifndef _WIN32
    cpu_set_t cpu_set;
    cpu_mask.computeNativeForm(cpu_set);
    int pret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set);
    switch (pret) {
    case 0:
        break; // no error
    case EFAULT:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                 "Bad memory address used for pthread_setaffinity_np");
    case EINVAL:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                 "Bad affinity mask for the worker thread");
    case ESRCH:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                 "Bad thread handle in pthread_setaffinity_np");
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                 << "Unhandled error while calling pthread_setaffinity_np: "
                                 << pret);
    }
#else
    // FIXME: More than 64 threads on Windows aren't support by that function.
    DWORD_PTR cpu_set;
    cpu_mask.computeNativeForm(cpu_set);
    DWORD_PTR ret = SetThreadAffinityMask(GetCurrentThread(), cpu_set);
    if (ret == 0) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                 << "SetThreadAffinityMask has failed. GetLastError return: "
                                 << StringBuilder::hex(GetLastError()));
    }

#endif // _WIN32
    uint64_t num = HSTR_CPU_MASK_COUNT(cpu_mask.mask);

    // Set up affinitized OpenMP on this stream, based on this streams CPUmask
    if (hStreams_GetOptions_openmp_policy() == HSTR_OPENMP_PRE_SETUP) {
        hStreams_init_partition(
            (uint64_t)0,
            (uint64_t)24, /* logical stream ID, used only for printing on sink FIXME?*/
            num,
            cpu_mask.mask[0],
            cpu_mask.mask[1],
            cpu_mask.mask[2],
            cpu_mask.mask[3],
            cpu_mask.mask[4],
            cpu_mask.mask[5],
            cpu_mask.mask[6],
            cpu_mask.mask[7],
            cpu_mask.mask[8],
            cpu_mask.mask[9],
            cpu_mask.mask[10],
            cpu_mask.mask[11],
            cpu_mask.mask[12],
            cpu_mask.mask[13],
            cpu_mask.mask[14],
            cpu_mask.mask[15]);
    }
}
}

worker_return_type hStreams_HostSideSinkWorker::workerMainLoop(void *ptr)
{
    hStreams_HostSideSinkWorker *worker = (hStreams_HostSideSinkWorker *) ptr;
    try {
        worker->worker_status_ = HSTR_RESULT_SUCCESS;
        set_affinity(worker->cpu_mask_);

        while (true, true) {
            std::unique_ptr<Action> action(worker->queue_->popFront());

            if (action.get() == NULL) {
                HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Host stream worker received an empty action.";

                continue;
            }

            if (action->getActionType() == COMPUTE) {
                std::unique_ptr<ComputePayload> &payload = action->getComputePayload();

                //Wait for all input_deps
                HSTR_COIRESULT result = hStreams_COIWrapper::COIEventWait((int16_t) payload->input_deps_.size(),
                                        &payload->input_deps_[0], -1, true, NULL, NULL);
                if (result != HSTR_COI_SUCCESS) {
                    HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                            << "Error while waiting on the input dependencies of an action: "
                            << hStreams_COIWrapper::COIResultGetName(result);

                    // skip the action
                    continue;
                }

                // Compute
                // Historically, we never handled any errors from the thunk
                hStreamsThunk(1, NULL, NULL, &(payload->args_[0]),
                              (uint16_t)(payload->args_.size()) * sizeof(uint64_t),
                              payload->ret_val_, payload->ret_val_size_);

                //Signal ret event
                result = hStreams_COIWrapper::COIEventSignalUserEvent(payload->ret_event_);
                if (result != HSTR_COI_SUCCESS) {
                    HSTR_ERROR(HSTR_INFO_TYPE_SYNC)
                            << "Error while signaling the output event of an action: "
                            << hStreams_COIWrapper::COIResultGetName(result);
                }

            } else if (action->getActionType() == STOP) {
                break;
            }
        }
    } catch (...) {
        worker->worker_status_ = hStreams_handle_exception();
    }
#ifndef _WIN32
    pthread_exit(NULL);
#else
    return 0;
#endif // _WIN32
}
