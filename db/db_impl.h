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
#include "port/thread_annotations.h"

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

        //
        //
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

        // 每一层的compaction状态
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

        Iterator *NewInternalIterator(const ReadOptions &read_options,
                                      SequenceNumber *latest_snapshot, uint32_t *seed);

        Status NewDB();

        Status Recover(VersionEdit *edit, bool *save_manifest) EXCLUSIVE_LOCKS_REQUIRED(mutex_) ;

        void MaybeIgnoreError(Status *s) const;

        void RemoveObsoleteFiles() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        void CompactMemTable() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        Status RecoverLogFile(uint64_t log_number,
                              bool last_log,
                              bool *save_manifest,
                              VersionEdit *edit,
                              SequenceNumber *max_sequence) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        Status WriteLevel0Table(MemTable *mem, VersionEdit *edit, Version *base) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        Status MakeRoomForWrite(bool force) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        WriteBatch *BuildBatchGroup(Writer **last_writer) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        void RecordBackgroundError(const Status &s) ;

        void MaybeScheduleCompaction() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        static void BGWork(void *db);

        void BackgroundCall();

        void BackgroundCompaction() EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        void CleanupCompaction(CompactionState *compact) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        Status DoCompactionWork(CompactionState *compact) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        Status OpenCompactionOutputFile(CompactionState *compact);

        Status FinishCompactionOutputFile(CompactionState *compact, Iterator *input);

        Status InstallCompactionResults(CompactionState *compact) EXCLUSIVE_LOCKS_REQUIRED(mutex_);

        const Comparator *user_comparator() const {
            return internal_comparator_.user_comparator();
        }

    private:
        Env *const env_;
        const InternalKeyComparator internal_comparator_;
        // const InternalFilterPolicy internal_filter_policy_;
        const Options options_;
        const bool owns_info_log_;
        const bool owns_cache_;
        const std::string dbname_;

        TableCache *const table_cache_;
        FileLock *db_lock_;

        port::Mutex mutex_;
        std::atomic<bool> shutting_down_;
        port::CondVar background_work_finish_signal_ GUARDED_BY(mutex_);
        MemTable *mem_;
        MemTable *imm_ GUARDED_BY(mutex_);
        std::atomic<bool> has_imm_;
        WritableFile *logfile_;
        uint64_t logfile_number_ GUARDED_BY(mutex_);
        log::Writer *log_;
        uint32_t seed_ GUARDED_BY(mutex_);

        std::deque<Writer *> writers_ GUARDED_BY(mutex_);
        WriteBatch *tmp_batch_ GUARDED_BY(mutex_);
        SnapshotList snapshots_ GUARDED_BY(mutex_);

        std::set<uint64_t> pending_outputs_ GUARDED_BY(mutex_);

        // 后台的compaction是否真正运行?
        bool background_compaction_scheduled_ GUARDED_BY(mutex_);

        ManualCompaction *manual_compaction_ GUARDED_BY(mutex_);

        VersionSet *const versions_ GUARDED_BY(mutex_);
        Status bg_error_ GUARDED_BY(mutex_);
        CompactionStats stats_[config::kNumLevels] GUARDED_BY(mutex_);
    };

    // Sanitize db options.  The caller should delete result.info_log if
    // it is not equal to src.info_log.
    // Options SanitizeOptions(const std::string& db,
    //                        const InternalKeyComparator* icmp,
    //                        const InternalFilterPolicy* ipolicy,
    //                        const Options& src);
}

#endif //MY_LEVELDB_DB_IMPL_H
