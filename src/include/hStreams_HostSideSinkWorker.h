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

#ifndef HSTREAMS_HSTREAMS_HOSTSIDESINKWORKER_H
#define HSTREAMS_HSTREAMS_HOSTSIDESINKWORKER_H

#include <vector>
#include <queue>
#include <memory>

#include "hStreams_types.h"
#include "hStreams_locks.h"
#include "hStreams_exceptions.h"
#include "hStreams_threading.h"

/// @brief Types of actions supported by the host-side streams worker
enum ACTION_TYPE {
    /// @brief Perform a computation.
    /// @sa hStreams_EnqueueCompute
    COMPUTE,
    /// @brief The host-side streams worker thread should exit
    STOP
};

/// @brief "Metadata" used by the \c COMPUTE action
///
/// Instances of this class carry the information necessary to perform
/// the computation on the "sink"
struct ComputePayload {
    /// @brief Arguments to supply to the sink-side function to be executed.
    ///
    /// The last element in this vector is the address of the function to be executed.
    std::vector<uint64_t> args_;
    /// @brief A collection of the events this action should depend on before it can execute
    std::vector<HSTR_EVENT> input_deps_;
    /// @brief Memory that the user functions can write to
    void *ret_val_;
    /// @brief Size of the memory the user functions can write to
    uint16_t ret_val_size_;
    /// @brief Custom event to be signaled once the computation finishes
    ///
    /// This event has to be registered through a call to \c COIEventRegisterUserEvent
    HSTR_EVENT ret_event_;

    /// @brief a helper constructor which will set up the internals of the struct.
    ComputePayload(std::vector<uint64_t> &args,
                   std::vector<HSTR_EVENT> &input_deps,
                   void *ret_val,
                   uint16_t &ret_val_size,
                   HSTR_EVENT ret_event);
};

class Action
{
    ACTION_TYPE action_type_;
    std::unique_ptr<ComputePayload> payload_;
public:

    //Action object is taking ownership of payload object
    Action(ACTION_TYPE action_type, std::unique_ptr<ComputePayload> payload);
    ~Action();

    std::unique_ptr<ComputePayload> &getComputePayload();

    ACTION_TYPE getActionType();
};

/// @brief A single consumer-single producer queue for the purpose of the host-side streams
///
/// Mimicking the behaviour of the queues in COI, this queue is an "infinite"
/// one. Obviously, in practive there are no infinite queues, all queues are
/// bounded by the resources of the operating system.
class hStreams_SPSCQueue
{
public:
    hStreams_SPSCQueue();
    ~hStreams_SPSCQueue();

    /// @brief Push a new action onto the queue
    /// @param action The action to be pushed onto the queue
    ///
    /// @note Through this method, the queue takes ownership of the action
    ///     object.  To provide an object to the queue you must therefore \c
    ///     move it into this method, e.g.:
    ///
    /// @code
    /// hStreams_SPSCQueue* myqueue = ...;
    /// std::unique_ptr<Action> myaction(...);
    /// myqueue.add(std::move(myaction));
    /// @endcode
    ///
    /// It might be helpful for the reader to understand the move semantics of C++11.
    void add(std::unique_ptr<Action> action);
    /// @brief Dequeue an element from the queue
    /// @returns an action element
    ///
    /// @note The queue releases the ownership of the action object, the agent
    ///     dequeuing the action will now own it.
    std::unique_ptr<Action> popFront();
private:
    /// @brief The underlying implementation of the queue.
    typedef std::queue<std::unique_ptr<Action> > queue_t;
    /// @brief Instantiation of the underlying queue implementation.
    queue_t queue_;
    /// @brief For synchronising the pushes/pops
    hStreams_Lock mutex_;
    /// @brief For synchronising the pushes/pops
    hStreams_CondVar cond_var_;
    // copy-ctor and assignment prohibited
    hStreams_SPSCQueue(hStreams_SPSCQueue const &other);
    hStreams_SPSCQueue &operator=(hStreams_SPSCQueue const &other);
};

// Implementation of a physical stream over COI, calls COIPipelineDestroy on
// the pipeline it has been created with upon the object's destruction
class hStreams_HostSideSinkWorker
{
public:
    hStreams_HostSideSinkWorker(hStreams_CPUMask const &cpu_mask);
    ~hStreams_HostSideSinkWorker();
    /// @brief Push a new action onto the worker's queue
    /// @param action The action to be pushed onto the queue
    /// @sa hStreams_SPSCQueue::add()
    ///
    /// It might be helpful for the reader to understand the move semantics of C++11.
    HSTR_RESULT putAction(std::unique_ptr<Action> action);
    /// @brief The bootstrap routine for the host-sink worker thread to execute
    static worker_return_type workerMainLoop(void *);
private:
    hStreams_CPUMask cpu_mask_;
    HSTR_RESULT worker_status_;
    hStreams_Lock lock_;
    std::unique_ptr<hStreams_SPSCQueue> queue_;
    std::unique_ptr<hStreams_Thread> thread_;
};

#endif /* HSTREAMS_HOSTSIDESINKWORKER_H */
