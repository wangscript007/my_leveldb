//
// Created by kuiper on 2021/2/20.
//

#ifndef MY_LEVELDB_DB_IMPL_H
#define MY_LEVELDB_DB_IMPL_H

#include <atomic>
#include <deque>
#include <set>
#include <string>

#include "db/dbformat.h"
#include "db/log_writer.h"
#include "db/snapshot.h"
#include "leveldb/db.h"
#include "leveldb/env.h"
#include "port/port.h"
#include "util/copyable.h"

namespace leveldb {

    class MemTable;

    class TableCache;

    class Version;

    class VersionEdit;

    class VersionSet;

    class DBImpl : public DB, public Copyable<false> {
    public:
        DBImpl(const Options &options, const std::string &dbname);

        ~DBImpl() override;


        // DB接口中定义的方法实现.
        Status Put(const WriteOptions &options, const Slice &key, const Slice &value) override;

        Status Delete(const WriteOptions &options, const Slice &key) override;

        Status Write(const WriteOptions &options, WriteBatch *updates) override;

        Status Get(const ReadOptions &options, const Slice &key, std::string *value) override;

        Iterator *NewIterator(const ReadOptions &options) override;

        const Snapshot *GetSnapshot() override;

        void ReleaseSnapshot(const Snapshot *snapshot) override;

        bool GetProperty(const Slice &property, std::string *value) override;

        void GetApproximateSizes(const Range *range, int n, uint64_t *sizes) override;

        void CompactRange(const Slice *begin, const Slice *end) override;

        // 额外用户做测试的方法

        void TEST_CompactRange(int level, const Slice *begin, const Slice *end);

        void TEST_CompactMemTable();

        Iterator *TEST_NewInternalIterator();

        int64_t TEST_MaxNextLevelOverlappingBytes();

        void RecordReadSample(Slice key);

    private:
        friend class DB;

        struct CompactionState;
        struct Writer;

        // 手工compaction的信息.
        struct ManualCompaction {
            int level;
            bool done;
            const InternalKey *begin;   // null代表是keyRange的第一个
            const InternalKey *end;     // null代表是keyRange的末一个
            InternalKey tmp_storage;
        };

        //
        struct CompactionStats {
            CompactionStats()
                    : micros(0), bytes_read(0), bytes_written(0) {
            }

            void Add(const CompactionStats &c) {
                this->micros += c.micros;
                this->bytes_read += c.bytes_read;
                this->bytes_written += c.bytes_written;
            }

            int64_t micros;
            int64_t bytes_read;
            int64_t bytes_written;
        };

    };
}

#endif //MY_LEVELDB_DB_IMPL_H
