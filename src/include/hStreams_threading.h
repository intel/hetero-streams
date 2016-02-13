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

#ifndef HSTREAMS_THREADING_H
#define HSTREAMS_THREADING_H

#include "hStreams_helpers_source.h"
#include "hStreams_exceptions.h"
#include <functional>

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

// Define one type for pthread and WINAPI threads worker
#ifndef _WIN32
typedef void *worker_return_type;
#else
typedef DWORD worker_return_type;
#endif

/// @brief A cross-platform implementation of a thread
class hStreams_Thread
{
#ifndef _WIN32
    pthread_t handle_;
#else
    HANDLE handle_;
#endif
public:
    /// @brief Create and start the thread
    /// @param start_routine The function to start executing
    /// @param arg The argument to pass to the function to be executed
    /// @throws hStreams_exception
    hStreams_Thread(worker_return_type(*start_routine)(void *), void *arg);
    /// @brief Wait for thread termination
    void join();
private:
    // copy-construction/assignment prohibited
    hStreams_Thread &operator=(const hStreams_Thread &other);
    hStreams_Thread(const hStreams_Thread &other);
};

// forward declaration
class hStreams_Lock;
/// @brief A cross-platform implementaion of a conditional variable
class hStreams_CondVar
{
#ifndef _WIN32
    pthread_cond_t cond_var_;
#else
    CONDITION_VARIABLE cond_var_;
#endif
public:
    hStreams_CondVar();
    ~hStreams_CondVar();
    /// @param c - is a callable predicate that shall signify that
    /// the wait on the variable should happen (true == wait)
    /// The mutex has to be locked
    void wait(hStreams_Lock &mutex, std::function<bool()> const &predicate);
    /// This method doesn't lock the mutex
    void signal();
private:
    hStreams_CondVar(hStreams_CondVar const &other);
    hStreams_CondVar &operator=(hStreams_CondVar const &other);
};


#endif /* HSTREAMS_THREADING_H */
