//
// Created by kuiper on 2021/2/7.
//

#ifndef MY_LEVELDB_COMPARATOR_H
#define MY_LEVELDB_COMPARATOR_H

#include "leveldb/export.h"
#include <string>

namespace leveldb {
    class Slice;

    class LEVELDB_EXPORT Comparator {
    public:

        // @brief
        virtual ~Comparator() = default;

        // @brief
        // @param
        // @param
        // @return
        virtual int Compare(const Slice &a, const Slice &b) const = 0;

        // @brief 获取当前比较器的名字.
        // @return 当前比较器的名字
        virtual const char *Name() const = 0;

        //
        //
        //
        virtual void FindShortestSeparator(std::string *start, const Slice &limit) const = 0;

        //
        //
        //
        virtual void FindShortSuccessor(std::string *key) const = 0;
    };

    LEVELDB_EXPORT const Comparator *BytewiseComparator();
}

#endif //MY_LEVELDB_COMPARATOR_H
