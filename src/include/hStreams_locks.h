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
#ifndef __HSTR_LOCKS_H__
#define __HSTR_LOCKS_H__

// Forward declaration for making it a friend of the lock class
class hStreams_CondVar;

// hStreams_Lock is a simple lock mechanism.  Note that all of the messy
// non-portable code is left to the implementation.
class hStreams_Lock
{
    friend class hStreams_CondVar;
public:
    // Public constructor initializes the lock.
    hStreams_Lock();

    // Public Deconstructor destroys the lock.
    ~hStreams_Lock();

    // Attempts to lock the lock.  If it is already locked, the thread blocks until
    // the lock is unlocked.
    // Returns 0 on success, else on non-zero and error occurred.
    int Lock(void);

    // Unlocks the lock.
    int Unlock(void);
private:
    // hiding copy constructor as private with no implementation
    hStreams_Lock(const hStreams_Lock &that);
    // hiding assignment operator as private with no implementation
    hStreams_Lock &operator= (const hStreams_Lock &that);
    void *mPrivateData;
};

// hStreams_Scope_Locker_Unlocker is a simple lock mechanism to augment the hStreams_Lock class declared above.
// This class allows writing critical section code in blocks delimted with '{' and '}', with automatic
// locking and unlocking of the mutex, ala:
//
// {
//    hStreams_Scope_Locker_Unlocker(hstreamslock);
//    // Start of critical section
//    // only one thread will execute here at a time.
//    ...
// }
// At the above brace, the deconstructor will automatically unlock the hstreamslock.
//

class hStreams_Scope_Locker_Unlocker
{
public:
    // Public constructor locks the given lock.
    hStreams_Scope_Locker_Unlocker(hStreams_Lock &);

    // Public Deconstructor unlocks the lock.
    ~hStreams_Scope_Locker_Unlocker();

private:
    // The default constructor is private and not defined:
    hStreams_Scope_Locker_Unlocker();
    // The copy constructor is private and not defined:
    hStreams_Scope_Locker_Unlocker(const hStreams_Scope_Locker_Unlocker &);
    // Assignment operator is declared private and not defined:
    hStreams_Scope_Locker_Unlocker &operator =(const hStreams_Scope_Locker_Unlocker &);

    hStreams_Lock &mLock;
};

// hStreams_RW_Lock is a simple readers/writer lock mechanism.  Note that all of the messy
// non-portable code is left to the implementation.
//
// Please note that due to the Windows implementation recursive rw_locks are not supported, and
// in addition, a read lock can not be converted into a write lock.
class hStreams_RW_Lock
{
public:
    // Public constructor initializes the lock.
    hStreams_RW_Lock();

    // Public Deconstructor destroys the lock.
    ~hStreams_RW_Lock();

    enum LockType {
        HSTR_RW_UNLOCKED,
        HSTR_RW_LOCK_READ,
        HSTR_RW_LOCK_WRITE,
    };

    // Attempts to lock the lock with either a READ or a WRITE lock.
    // The usual reader/writer rules apply for the hStreams_RW_Lock:
    // 1. If the lock is not locked for read or write and a read  lock is requested, it is granted immediately.
    // 2. If the lock is not locked for read or write and a write lock is requested, it is granted immediately.
    // 3. If the lock is     locked for reading       and a read  lock is requested, it is granted immediately.
    // 4. If the lock is     locked for reading       and a write lock is requested, it will block until all read locks are unlocked.
    // 5. If the lock is     locked for writing       and a read  lock is requested, it will block until the write lock is unlocked.
    // 5. If the lock is     locked for writing       and a write lock is requested, it will block until the write lock is unlocked.
    int Lock(LockType);

    // Unlocks the lock.

    int Unlock(LockType);

private:
    // hiding copy constructor as private with no implementation
    hStreams_RW_Lock(const hStreams_RW_Lock &that);
    // hiding assignment operator as private with no implementation
    hStreams_RW_Lock &operator= (const hStreams_RW_Lock &that);

    void *mPrivateData;
};

// hStreams_RW_Scope_Locker_Unlocker is a simple lock mechanism to augment the hStreams_RW_Lock class declared above.
// This class allows writing critical section code in blocks delimited by '{' and '}', with automatic
// locking and unlocking of the lock, ala:
//
// {
//    hStreams_RW_Scope_Locker_Unlocker(hstreamsrwlock, LockType );
//    // Start of critical section
//    // only one thread will execute here at a time.
//    ...
// }
// At the above brace, the deconstructor will automatically unlock the hstreamslock.
//

class hStreams_RW_Scope_Locker_Unlocker
{
public:
    // Public constructor locks the given lock with the given lock type.
    hStreams_RW_Scope_Locker_Unlocker(hStreams_RW_Lock &, hStreams_RW_Lock::LockType);

    // Public Deconstructor unlocks the lock.
    ~hStreams_RW_Scope_Locker_Unlocker();

private:
    // The default constructor is declared private and not defined:
    hStreams_RW_Scope_Locker_Unlocker();
    // The copy constructor is declared private and not defined:
    hStreams_RW_Scope_Locker_Unlocker(const hStreams_RW_Scope_Locker_Unlocker &);
    // Assignment operator is declared private and not defined.
    hStreams_RW_Scope_Locker_Unlocker &operator =(const hStreams_RW_Scope_Locker_Unlocker &);

    hStreams_RW_Lock &mRWLock;
    const hStreams_RW_Lock::LockType mLockType;
};

#endif  /* __HSTR_LOCKS_H__ */
