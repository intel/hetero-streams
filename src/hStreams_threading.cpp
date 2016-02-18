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

#include "hStreams_types.h"
#include "hStreams_helpers_source.h"
#include "hStreams_exceptions.h"
#include "hStreams_locks.h"
#include "hStreams_threading.h"
#include "hStreams_internal.h"


hStreams_Thread::hStreams_Thread(worker_return_type(*start_routine)(void *), void *arg)
{
#ifndef _WIN32

    int pret = pthread_create(&handle_, NULL, start_routine, arg);
    switch (pret) {
    case 0:
        break;
    case EAGAIN:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_RESOURCE_EXHAUSTED,
                                   "Insufficient resources to create another thread");
    case EINVAL:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "Wrong arguments supplied to thread creation");
    case EPERM:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "Insufficient permissions to create a thread with specified attributes");
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Unhandled error while calling pthread_create: "
                                   << pret);
    }
#else
    handle_ = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
    if (handle_ == NULL) {
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "CreateThread has failed. GetLastError return: "
                                   << StringBuilder::hex(GetLastError()));
    }
#endif
}

void hStreams_Thread::join()
{
#ifndef _WIN32
    int pret = pthread_join(handle_, NULL);
    switch (pret) {
    case 0:
        break;
    case EDEADLK:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "A deadlock was detected while trying to join a thread");
    case EINVAL:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "Tried to join a non-joinable thread or another thread"
                                   "is already waiting to join with this thread");
    case ESRCH:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "Tried to join with a wrong thread handle");
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Unhandled error while calling pthread_join: "
                                   << pret);
    }
#else
    DWORD ret = WaitForSingleObject(handle_, INFINITE);
    switch (ret) {
    case 0:
        break;
    case WAIT_ABANDONED:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "A deadlock was detected while calling WaitForSingleObject");
    case WAIT_TIMEOUT:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "WaitForSingleObject time out reached.");
    case WAIT_FAILED:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "WaitForSingleObject has failed. GetLastError return: "
                                   << StringBuilder::hex(GetLastError()));
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Unhandled error while calling WaitForSingleObject: "
                                   << ret);
    }
#endif
}

hStreams_CondVar::hStreams_CondVar()
{
#ifndef _WIN32
    int pret = pthread_cond_init(&cond_var_, NULL);
    switch (pret) {
    case 0:
        break;
    case ENOMEM:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_OUT_OF_MEMORY,
                                   "Not enough resources to initialize the conditional variable");
    case EAGAIN:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_RESOURCE_EXHAUSTED,
                                   "The operating system ran out of resources while trying to"
                                   "initialize the conditional variable");
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Unhandled error while calling pthread_cond_init: "
                                   << pret);
    }
#else
    InitializeConditionVariable(&cond_var_);
#endif
}

hStreams_CondVar::~hStreams_CondVar()
{
#ifndef _WIN32
    // It's the destructor, we have to handle failures gracefully
    try {
        int pret = pthread_cond_destroy(&cond_var_);
        switch (pret) {
        case 0:
            break;
        case EBUSY:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                       "Trying to destroy a conditional variable while it is being used");
        case EINVAL:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                       "Trying to destroy an invalid conditional variable");
        default:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                       << "Unhandled error while calling pthread_cond_destroy: "
                                       << pret);
        }
    } catch (...) {
        hStreams_handle_exception();
    }
#endif
}

void hStreams_CondVar::signal()
{
#ifndef _WIN32
    int pret = pthread_cond_signal(&cond_var_);
    switch (pret) {
    case 0:
        break;
    case EINVAL:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                   "Trying to signal an unitialised condition variable");
    default:
        throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                   << "Unhandled error while calling pthread_cond_signal: "
                                   << pret);
    }
#else
    WakeConditionVariable(&cond_var_);
#endif
}

void hStreams_CondVar::wait(hStreams_Lock &mutex, std::function<bool()> const &predicate)
{
#ifndef _WIN32
    while (predicate()) {
        int pret = pthread_cond_wait(&cond_var_, (pthread_mutex_t *)mutex.mPrivateData);
        switch (pret) {
        case 0:
            break;
        case EINVAL:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                       "Invalid handle passed to pthread_cond_wait()");
        case EPERM:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR,
                                       "Try to lock a mutex not owned by the current thread");
        default:
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                       << "Unhandled error while calling pthread_cond_wait: "
                                       << pret);
        }
    }
#else
    while (predicate()) {
        BOOL res = SleepConditionVariableCS(&cond_var_, (LPCRITICAL_SECTION) mutex.mPrivateData, INFINITE);
        if (!res) {
            throw HSTR_EXCEPTION_MACRO(HSTR_RESULT_INTERNAL_ERROR, StringBuilder()
                                       << "SleepConditionVariableCS has failed. GetLastError return: "
                                       << StringBuilder::hex(GetLastError()));
        }
    }
#endif
}

