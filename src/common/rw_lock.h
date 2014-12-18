/*
 * rw_lock.h
 *
 *  Created on: May 8, 2014
 *      Author: wanghuan
 */

#ifndef RW_LOCK_H_
#define RW_LOCK_H_

enum RWLockPrefer
{
    RWLockReadPrefer,
    RWLockWritePrefer
};

class RWLock
{
public:
    RWLock(RWLockPrefer mode = RWLockWritePrefer)
    {
        pthread_rwlockattr_init(&mAttr);

        if (mode == RWLockWritePrefer)
        {
            pthread_rwlockattr_setkind_np(&mAttr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        }
        else
        {
            pthread_rwlockattr_setkind_np(&mAttr, PTHREAD_RWLOCK_PREFER_READER_NP);
        }
        pthread_rwlock_init(&mRwLock, &mAttr);
    }

    ~RWLock()
    {
        pthread_rwlock_destroy(&mRwLock);
    }

public:
    inline void read_lock()
    {
        pthread_rwlock_rdlock(&mRwLock);
    }

    inline void unlock()
    {
        pthread_rwlock_unlock(&mRwLock);
    }

    inline void write_lock()
    {
        pthread_rwlock_wrlock(&mRwLock);
    }

private:
    ::pthread_rwlock_t mRwLock;
    ::pthread_rwlockattr_t mAttr;
};

enum RWLockMode
{
    RWLOCK_READ,
    RWLOCK_WRITE
};

class RWLockScopedGuard
{
public:
    RWLockScopedGuard(RWLock& lock, const RWLockMode& mode) : mLock(lock)
    {
        if (mode == RWLOCK_READ)
        {
            mLock.read_lock();
        }
        else
        {
            mLock.write_lock();
        }
    }

    ~RWLockScopedGuard()
    {
        mLock.unlock();
    }

private:
    RWLock& mLock;
};

#endif /* RW_LOCK_H_ */
