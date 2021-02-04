//
// Created by kuiper on 2021/2/4.
//

#ifndef MY_LEVELDB_PORT_STDCXX_H
#define MY_LEVELDB_PORT_STDCXX_H

#include <mutex>
#include <condition_variable>

namespace leveldb {
    namespace port {

        class CondVar;

        class Mutex {
        public:
            Mutex() = default;

            ~Mutex() = default;

            Mutex(const Mutex &) = delete;

            Mutex &operator=(const Mutex &) = delete;

            void Lock() {
                mtx_.lock();
            }

            bool TryLock() {
                return mtx_.try_lock();
            }

            void Unlock() {
                mtx_.unlock();
            }

            void AssertHeld() {
                (void) mtx_;
            }

        private:
            friend class CondVar;

            std::mutex mtx_;
        };


        class CondVar {
        public:
            explicit CondVar(Mutex *pMutex)
                    : pMutex_(pMutex) {
                assert(pMutex != nullptr);
            }

            ~CondVar() = default;

            CondVar(const CondVar &) = delete;
            CondVar &operator=(CondVar &) = delete;

            void Wait() {
                pMutex_->AssertHeld();
                std::unique_lock<std::mutex> lock(pMutex_->mtx_, std::adopt_lock);
                cv_.wait(lock);
                lock.release();
            }

            void Signal() { cv_.notify_one(); }

            void SignalAll() { cv_.notify_all(); }

        private:
            Mutex *const pMutex_;
            std::condition_variable cv_;
        };

    }
}

#endif //MY_LEVELDB_PORT_STDCXX_H
