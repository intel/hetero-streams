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
#include "hStreams_locks.h"
#include "hStreams_common.h"
#include "hStreams_internal.h"
#include "hStreams_Logger.h"

#ifndef _WIN32
#include <pthread.h>
#define HSTR_LOCK    pthread_mutex_t
#define HSTR_RW_LOCK pthread_rwlock_t
#else
#include <Windows.h>
#define HSTR_LOCK    CRITICAL_SECTION
#define HSTR_RW_LOCK SRWLOCK
#endif

// Constructor of hStreams_Lock class:
hStreams_Lock::hStreams_Lock()
{
    HSTR_LOCK *tmp = new HSTR_LOCK;
#ifndef _WIN32
    int rv;
    if (rv = pthread_mutex_init(tmp, NULL)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot initialize mutex, code: " << rv;
    }
#else
    InitializeCriticalSection(tmp);
#endif
    mPrivateData = (void *) tmp;
}

// Destructor of hStreams_Lock class:
hStreams_Lock::~hStreams_Lock()
{
    HSTR_LOCK *tmp = (HSTR_LOCK *) mPrivateData;
#ifndef _WIN32
    int rv = pthread_mutex_destroy(tmp);
#ifdef _DEBUG
    if (rv) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot destroy mutex, code: " << rv;
    }
#endif
#else
    DeleteCriticalSection(tmp);
#endif
    delete tmp;
}

// hStreams_Lock::Lock member function:
// returns non-zero on a failure.
int
hStreams_Lock::Lock()
{
    HSTR_LOCK *tmp = (HSTR_LOCK *) mPrivateData;
    int rv;
#ifndef _WIN32
    if (rv = pthread_mutex_lock(tmp)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot lock, code: " << rv;
    }
#else
    rv = 0;
    EnterCriticalSection(tmp);
#endif
    return rv;
}

// hStreams_Lock::Unlock member function:
// returns non-zero on a failure.
int
hStreams_Lock::Unlock()
{
    HSTR_LOCK *tmp = (HSTR_LOCK *) mPrivateData;
    int rv;
#ifndef _WIN32
    if (rv = pthread_mutex_unlock(tmp)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot unlock, code: " << rv;
    }
#else
    rv = 0;
    LeaveCriticalSection(tmp);
#endif
    return rv;
}

// Constructor for hStreams_Scope_Locker_Unlocker class:
hStreams_Scope_Locker_Unlocker::hStreams_Scope_Locker_Unlocker(hStreams_Lock &hsl): mLock(hsl)
{
    mLock.Lock();
}

// Destructor for hStreams_Scope_Locker_Unlocker class:
hStreams_Scope_Locker_Unlocker::~hStreams_Scope_Locker_Unlocker()
{
    mLock.Unlock();
}

// Constructor for hStreams_RW_Lock class:
hStreams_RW_Lock::hStreams_RW_Lock()
{
    HSTR_RW_LOCK *tmp = new HSTR_RW_LOCK;

#ifndef _WIN32
    int rv;

    if (rv = pthread_rwlock_init(tmp, NULL)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot initialize rwlock, code: " << rv;
    }
#else
    InitializeSRWLock(tmp);
#endif

    mPrivateData = (void *) tmp;
}

// Destructor for hStreams_RW_Lock class:
hStreams_RW_Lock::~hStreams_RW_Lock()
{
    HSTR_RW_LOCK *tmp = (HSTR_RW_LOCK *) mPrivateData;

#ifndef _WIN32
    int rv = pthread_rwlock_destroy(tmp);

#ifdef _DEBUG
    if (rv) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot destroy rwlock, code: " << rv;
    }
#endif
#else
    // Nothing to do here.
#endif

    delete tmp;
}

// hStreams_RW_Lock::Lock member function:
int
hStreams_RW_Lock::Lock(LockType lt)
{
    HSTR_RW_LOCK *tmp = (HSTR_RW_LOCK *) mPrivateData;
    int rv;

#ifndef _WIN32
    int (*func)(HSTR_RW_LOCK *) = (lt == HSTR_RW_LOCK_READ) ? pthread_rwlock_rdlock : pthread_rwlock_wrlock;


    if (rv = func(tmp)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot lock, code: ";
    }
#else
    switch (lt) {
    case HSTR_RW_LOCK_READ:
        AcquireSRWLockShared(tmp);
        break;

    case HSTR_RW_LOCK_WRITE:
        AcquireSRWLockExclusive(tmp);
        break;
    }
    rv = 0;

#endif

    return rv;
}

// hStreams_RW_Lock::Unlock member function:
int
hStreams_RW_Lock::Unlock(LockType lt)
{
    HSTR_RW_LOCK *tmp = (HSTR_RW_LOCK *) mPrivateData;
    int rv;

#ifndef _WIN32

    if (rv = pthread_rwlock_unlock(tmp)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC) << "Cannot unlock, code: " << rv;
    }
#else
    switch (lt) {
    case HSTR_RW_LOCK_READ:
        ReleaseSRWLockShared(tmp);
        break;

    case HSTR_RW_LOCK_WRITE:
        ReleaseSRWLockExclusive(tmp);
        break;
    }
    rv = 0;

#endif

    return rv;
}

// Constructor for hStreams_RW_Scope_Locker_Unlocker class:
hStreams_RW_Scope_Locker_Unlocker::hStreams_RW_Scope_Locker_Unlocker(hStreams_RW_Lock &hsl, hStreams_RW_Lock::LockType lt):
    mRWLock(hsl), mLockType(lt)
{
    mRWLock.Lock(mLockType);
}

// Destructor for hStreams_RW_Scope_Locker_Unlocker class:
hStreams_RW_Scope_Locker_Unlocker::~hStreams_RW_Scope_Locker_Unlocker()
{
    mRWLock.Unlock(mLockType);
}
