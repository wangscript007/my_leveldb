//
// Created by kuiper on 2021/2/20.
//

#include <atomic>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "db/db_impl.h"
#include "db/filename.h"
#include "db/log_reader.h"
#include "db/log_writer.h"
#include "db/memtable.h"
#include "db/version_set.h"
#include "db/write_batch_internal.h"

#include "leveldb/db.h"
#include "leveldb/env.h"
#include "leveldb/status.h"

#include "util/coding.h"
#include "util/logging.h"
#include "util/mutexlock.h"

namespace leveldb {
    const int kNumNonTableCacheFiles = 10;

    struct DBImpl::Writer {
        explicit Writer(port::Mutex *mu)
                : batch(nullptr), sync(false), done(false), cv(mu) {}

        Status status;
        WriteBatch *batch;
        bool sync;
        bool done;
        port::CondVar cv;
    };

    struct DBImpl::CompactionState {
        struct Output {
            uint64_t number;
            uint64_t file_size;
            InternalKey smallest, largest;
        };
        // COMPLETE ME !fixme
    };

    template<class T, class V>
    static void ClipToRange(T *ptr, V minvalue, V maxvalue) {
        if (static_cast<V>(*ptr) > maxvalue) *ptr = maxvalue;
        if (static_cast<V>(*ptr) < minvalue) *ptr = minvalue;
    }

    // FIXME SanitizeOptions

    static int TableCacheSize(const Options &sanitized_options) {
        return sanitized_options.max_open_files - kNumNonTableCacheFiles;
    }

    DBImpl::DBImpl(const Options &raw_options, const std::string &dbname)
            : env_(raw_options.env),
              internal_comparator_(raw_options.comparator),
            // filter_policy
            // options
              owns_info_log_(options_.info_log != raw_options.info_log),
              owns_cache_(options_.block_cache != raw_options.block_cache),
              dbname_(dbname),
              table_cache_(new TableCache(dbname_, options_, TableCacheSize(options_))),
              db_lock_(nullptr),
              shutting_down_(false),
              background_work_finish_signal_(&mutex_),
              mem_(nullptr),
              imm_(nullptr),
              has_imm_(false),
              logfile_(nullptr),
              logfile_number_(0),
              log_(nullptr),
              seed_(0),
              tmp_batch_(new WriteBatch),
              background_compaction_scheduled_(false),
              manual_compaction_(nullptr)
    // fixme versions_(new VersionSet)
    {

    }

    DBImpl::~DBImpl() {
        mutex_.Lock();
        shutting_down_.store(true, std::memory_order_release);
        while (background_compaction_scheduled_) {
            background_work_finish_signal_.Wait();
        }
        mutex_.Unlock();

        // 如果db上了锁,则解锁
        if (db_lock_ != nullptr) {
            env_->UnlockFile(db_lock_); // delete db_lock_ internal
        }

        delete versions_;
        if (mem_ != nullptr) mem_->Unref();
        if (imm_ != nullptr) imm_->Unref();
        delete tmp_batch_;
        delete log_;
        delete logfile_;
        delete table_cache_;

        if (owns_info_log_) {
            delete options_.info_log;
        }

        if (owns_cache_) {
            delete options_.block_cache;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    // DB::Put
    // DB::Delete
    // DB::Open
    // DestroyDB

    // 纯虚函数也可以有默认实现.
    Status DB::Put(const WriteOptions &options, const Slice &key, const Slice &value) {
        WriteBatch batch;
        batch.Put(key, value);
        return Write(options, &batch);
    }

    Status DB::Delete(const WriteOptions &options, const Slice &key) {
        WriteBatch batch;
        batch.Delete(key);
        return Write(options, &batch);
    }

    Status DB::Open(const Options &options, const std::string &name, DB **dbptr) {

    }

    Status DestroyDB(const std::string &dbname, const Options &options) {
        Env *env = options.env;
        std::vector<std::string> filenames;
        auto status = env->GetChildren(dbname, &filenames);
        if (!status.IsOK()) {
            // 目录都不存在则忽略这个错误.
            return Status::OK();
        }

        FileLock *lock;
        const std::string lock_name = LockFileName(dbname);
        status = env->LockFile(lock_name, &lock);
        if (status.IsOK()) {
            uint64_t number;
            FileType type;
            for (int i = 0; i < filenames.size(); ++i) {
                if (ParseFileName(filenames[i], &number, &type)
                    && type != FileType::kDBLockFile) {
                    Status del_status = env->RemoveFile(dbname + "/" + filenames[i]);
                    if (status.IsOK() && !del_status.IsOK()) {
                        status = del_status;
                    }
                }
            }

            env->UnlockFile(lock);
            env->RemoveFile(lock_name);
            env->RemoveDir(dbname);
        }
        return status;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////

}   // end of namespace levedb
