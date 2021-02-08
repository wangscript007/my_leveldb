//
// Created by kuiper on 2021/2/8.
//

#ifndef MY_LEVELDB_OPTIONS_H
#define MY_LEVELDB_OPTIONS_H

#include <cstddef>
#include "leveldb/export.h"

namespace leveldb {

    class Cache;

    class Comparator;

    class Env;

    class FilterPolicy;

    class Logger;

    class Snapshot;

    // 压缩类型
    enum CompressionType {
        kNoCompression = 0x0,           // 不压缩
        kSnappyCompression = 0x01,      // snappy压缩算法
    };

    struct LEVELDB_EXPORT Options {
        Options();

        const Comparator *comparator;

        bool create_if_missing = false;

        bool error_if_exists = false;

        bool paranoid_checks = false;

        Env *env;

        Logger *info_log = nullptr;

        size_t write_buffer_size = 1024 * 1024 * 4;

        int max_open_files = 1000;

        Cache* block_cache = nullptr;

        size_t block_size = 1024 * 4;

        int block_restart_interval = 16;

        size_t max_file_size = 1024 * 1024 * 2;

        CompressionType compressionType = kSnappyCompression;

        bool reuse_logs = false;

        const FilterPolicy* filter_policy = nullptr;
    };

    struct LEVELDB_EXPORT ReadOptions {
        ReadOptions() = default;

        bool verify_checksums = false;
        bool fill_cache = true;
        const Snapshot * snapshot = nullptr;
    };

    struct LEVELDB_EXPORT WriteOptions {
        WriteOptions() = default;

        bool sync = false;
    };


}

#endif //MY_LEVELDB_OPTIONS_H
