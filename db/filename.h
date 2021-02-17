//
// Created by kuiper on 2021/2/17.
//

#ifndef MY_LEVELDB_FILENAME_H
#define MY_LEVELDB_FILENAME_H

#include <cstdint>
#include <string>

#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "port/port.h"

namespace leveldb {
    class Env;

    // leveldb用到的所有文件类型
    enum FileType {
        kLogFile,
        kDBLockFile,
        kTableFile,
        kDescriptorFile,
        kCurrentFile,
        kTempFile,
        kInfoLogFile
    };

    extern std::string LogFileName(const std::string &dbname, uint64_t number);

    extern std::string TableFileName(const std::string &dbname, uint64_t number);

    extern std::string SSTTableFileName(const std::string &dbname, uint64_t number);

    extern std::string DescriptorFileName(const std::string &dbname, uint64_t number);

    extern std::string CurrentFileName(const std::string &dbname);

    extern std::string LockFileName(const std::string &dbname);

    extern std::string TempFileName(const std::string &dbname, uint64_t number);

    extern std::string InfoLogFileName(const std::string &dbname);

    extern std::string OldInfoLogFileName(const std::string &dbname);

    extern bool ParseFileName(const std::string &filename, uint64_t *number, FileType *type);

    extern Status SetCurrentFile(Env *env, const std::string &dbname, uint64_t descriptor_number);

}

#endif //MY_LEVELDB_FILENAME_H
