//
// Created by kuiper on 2021/2/16.
//

#ifndef MY_LEVELDB_MEMTABLE_H
#define MY_LEVELDB_MEMTABLE_H

#include "util/arena.h"
#include "db/skiplist.h"
#include "leveldb/db.h"

namespace leveldb {

    class InternalKeyComparator;
    class MemTableIterator;

    // MemTable基于引用计数.
    class MemTable {
    public:
        explicit MemTable(const InternalKeyComparator &comparator);

        // Disable copy and assign.
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

        // 获取大概的使用内存
        size_t ApproximateMemoryUsage();

        // Iterator* NewIterator();



    private:
        friend class MemTableIterator;
        friend class MemTableBackwardIterator;

        ~MemTable(); // 只有引用计数减少0才能删除

        struct KeyComparator {
            const InternalKeyComparator comparator;
            explicit KeyComparator(const InternalKeyComparator& c)
                : comparator(c) {}
        };

        using Table = SkipList<const char*, KeyComparator>;

        KeyComparator comparator_;
        int refs_;
        Arena arena_;
        Table table_;
    };

}

#endif //MY_LEVELDB_MEMTABLE_H
