//
// Created by kuiper on 2021/2/16.
//

#ifndef MY_LEVELDB_MEMTABLE_H
#define MY_LEVELDB_MEMTABLE_H

#include "util/arena.h"


namespace leveldb {

    class InternalKeyComparator;

    class MemTableIterator;

    // MemTable基于引用计数.
    class MemTable {
    public:
        explicit MemTable(const InternalKeyComparator &comparator);

        MemTable(const MemTable &) = delete;

        MemTable &operator=(const MemTable &) = delete;

        void Ref() {
            ++refs_;
        }

        void Unref() {
            --refs_;
            assert(refs_ >= 0);
            if (refs_ <= 0) {
                delete this;
            }
        }

        size_t ApproximateMemoryUsage();

    private:
        friend class MemTableIterator;

        friend class MemTableBackwardIterator;

        struct KeyComparator {

        };

        ~MemTable(); // 只有引用计数减少0才能删除

        KeyComparator comparator_;
        int refs_;
        Arena arena_;
        // Table
    };

}

#endif //MY_LEVELDB_MEMTABLE_H
