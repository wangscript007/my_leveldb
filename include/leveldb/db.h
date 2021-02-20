#ifndef MY_LEVELDB_DB_H
#define MY_LEVELDB_DB_H

#include <string>

#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/options.h"
#include "leveldb/status.h"
#include "leveldb/iterator.h"

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
        static Status Open(const Options& options, const std::string& name, DB** dbptr);

        DB() = default;
        DB(const DB&) = delete;
        DB& operator=(const DB&) = delete;

        virtual ~DB() = default;

        virtual Status Put(const WriteOptions& options, const Slice& key, const Slice &value) = 0;
        virtual Status Delete(const WriteOptions& options, const Slice& key) = 0;
        virtual Status Write(const WriteOptions& options, WriteBatch* updates) = 0;
        virtual Status Get(const ReadOptions& options, const Slice& key,
                           std::string* value) = 0;

        virtual Iterator* NewIterator(const ReadOptions& options) = 0;
        virtual const Snapshot* GetSnapshot() = 0;
        virtual void ReleaseSnapshot(const Snapshot* snapshot) = 0;

        virtual bool GetProperty(const Slice& property, std::string *value) = 0;
        virtual void GetApproximateSizes(const Range* range, int n, uint64_t* sizes) = 0;
        virtual void CompactRange(const Slice* begin, const Slice* end) = 0;
    };

    // 销毁整个数据库 db_impl.cc
    LEVELDB_EXPORT Status DestroyDB(const std::string& name, const Options& options);

    // 重新排序整个数据库
    LEVELDB_EXPORT Status RepairDB(const std::string &name, const Options & options);

}

#endif
