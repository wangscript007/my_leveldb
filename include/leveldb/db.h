#ifndef MY_LEVELDB_DB_H
#define MY_LEVELDB_DB_H

#include <string>

#include "leveldb/export.h"
#include "leveldb/slice.h"

namespace leveldb {

    // 如果修改了CMakeLists.txt，则这里也需要同步修改
    static const int kMajorVersion = 1;
    static const int kMinorVersion = 22;

    struct Options;
    struct ReadOptions;
    struct WriteOptions;

    class WriteBatch;

    class LEVELDB_EXPORT Snapshot {
    protected:
        virtual ~Snapshot() = default;
    };

    struct LEVELDB_EXPORT Range {
        Range() = default;
        Range(const Slice& s, const Slice &l) : start(s), limit(l) {}

        Slice start;    // Include
        Slice limit;    // Not Include
    };

    class LEVELDB_EXPORT DB {
    public:
        // TODO
    };

}

#endif
