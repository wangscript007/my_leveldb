//
// Created by kuiper on 2021/2/19.
//

#ifndef MY_LEVELDB_ITERATOR_H
#define MY_LEVELDB_ITERATOR_H

#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

    class LEVELDB_EXPORT Iterator {
    public:
        Iterator();

        Iterator(const Iterator &) = delete;

        Iterator &operator=(const Iterator &) = delete;

        virtual ~Iterator();

        virtual bool Valid() const = 0;

        virtual void SeekToFirst() = 0;

        virtual void SeekToLast() = 0;

        virtual void Seek(const Slice &target) = 0;

        virtual void Next() = 0;

        virtual void Prev() = 0;

        virtual Slice Key() const = 0;

        virtual Slice Value() const = 0;

        virtual Status status() const = 0;

        using CleanupFunction = void (*)(void *arg1, void *arg2);

        void RegisterCleanup(CleanupFunction func, void *arg1, void *arg2);

    private:
        struct CleanupNode {
            CleanupNode *next;
            CleanupFunction function;
            void *arg1;
            void *arg2;

            bool IsEmpty() const { return function == nullptr; }

            void Run() {
                assert(function != nullptr);
                function(arg1, arg2);
            }
        };

        CleanupNode cleanup_head_;
    };

    LEVELDB_EXPORT Iterator *NewEmptyIterator();
    LEVELDB_EXPORT Iterator *NewErrorIterator(const Status &status);

}

#endif //MY_LEVELDB_ITERATOR_H
