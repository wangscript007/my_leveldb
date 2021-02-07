//
// Created by kuiper on 2021/2/7.
//

#ifndef MY_LEVELDB_MUTEXLOCK_H
#define MY_LEVELDB_MUTEXLOCK_H

#include "port/port.h"

namespace leveldb {
    class MutexLock {
    public:
        explicit MutexLock(port::Mutex *mtx)
                : pMutex_(mtx) {
            pMutex_->Lock();
        }

        ~MutexLock() {
            pMutex_->Unlock();
        }

        MutexLock(const MutexLock &) = delete;

        MutexLock &operator=(const MutexLock &) = delete;

    private:
        port::Mutex *const pMutex_;
    };
}

#endif //MY_LEVELDB_MUTEXLOCK_H
