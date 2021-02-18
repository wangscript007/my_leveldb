//
// Created by kuiper on 2021/2/17.
//

#include "db/filename.h"
#include "leveldb/env.h"
#include "util/logging.h"

namespace leveldb {

    // util/env.cc
    extern Status WriteStringToFileSync(Env *env, const Slice &data, const std::string &fname);

    static std::string MakeFileName(const std::string &dbname, uint64_t number, const char *suffix) {
        char buf[100] = {0};
        std::snprintf(buf, sizeof(buf), "/%06llu.%s", static_cast<unsigned long long>(number), suffix);
        return dbname + buf;
    }

    // extern functions in filename.h

    std::string LogFileName(const std::string &dbname, uint64_t number) {
        assert(number > 0);
        return MakeFileName(dbname, number, "log");
    }

    std::string TableFileName(const std::string &dbname, uint64_t number) {
        assert(number > 0);
        return MakeFileName(dbname, number, "ldb");
    }

    std::string SSTTableFileName(const std::string &dbname, uint64_t number) {
        assert(number > 0);
        return MakeFileName(dbname, number, "sst");
    }

    std::string DescriptorFileName(const std::string &dbname, uint64_t number) {
        assert(number > 0);
        char buf[100] = {0};
        std::snprintf(buf, sizeof(buf), "/MANIFEST-%06llu", static_cast<unsigned long long>(number));
        return dbname + buf;
    }

    std::string CurrentFileName(const std::string &dbname) {
        return dbname + "/CURRENT";
    }

    std::string LockFileName(const std::string &dbname) {
        return dbname + "/LOCK";
    }

    std::string TempFileName(const std::string &dbname, uint64_t number) {
        assert(number > 0);
        return MakeFileName(dbname, number, "dbtmp");
    }

    std::string InfoLogFileName(const std::string &dbname) {
        return dbname + "/LOG";
    }

    std::string OldInfoLogFileName(const std::string &dbname) {
        return dbname + "/LOG.old";
    }

    //  尝试解析文件名得到相关的信息
    //  dbname/CURRENT
    //  dbname/LOCK
    //  dbname/LOG
    //  dbname/LOG.old
    //  dbname/MANIFEST-[0-9]+
    //  dbname/[0-9]+.(log|sst|ldb|dbtmp)
    bool ParseFileName(const std::string &filename, uint64_t *number, FileType *type) {
        Slice rest(filename);
        if (rest == "CURRENT") {
            *number = 0;
            *type = kCurrentFile;
        } else if (rest == "LOCK") {
            *number = 0;
            *type = kDBLockFile;
        } else if (rest == "LOG" || rest == "LOG.old") {
            *number = 0;
            *type = kInfoLogFile;
        } else if (rest.starts_with("MANIFEST-")) {
            rest.remove_prefix(strlen("MANIFEST-"));
            uint64_t num;
            if (!ConsumeDecimalNumber(&rest, &num)) {
                return false;
            }
            if (!rest.empty()) {
                return false;
            }
            *type = kDescriptorFile;
            *number = num;
        } else {
            uint64_t num;
            if (!ConsumeDecimalNumber(&rest, &num)) {
                return false;
            }
            Slice suffix = rest;
            if (suffix == ".log") {
                *type = kLogFile;
            } else if (suffix == ".sst" || suffix == ".ldb") {
                *type = kTableFile;
            } else if (suffix == ".dbtmp") {
                *type = kTempFile;
            } else {
                return false;
            }
            *number = num;
        }

        return true;
    }

    Status SetCurrentFile(Env *env, const std::string &dbname, uint64_t descriptor_number) {
        std::string manifest = DescriptorFileName(dbname, descriptor_number);
        Slice contents = manifest;
        assert(contents.starts_with(dbname + "/"));
        contents.remove_prefix(dbname.size() + 1);
        // contents likes: MANIFEST-1111

        std::string tmp = TempFileName(dbname, descriptor_number);
        // tmp likes dbname/1111.dbtmp

        Status s = WriteStringToFileSync(env, contents.ToString() + "\n", tmp);
        if (s.IsOK()) {
            s = env->RenameFile(tmp, CurrentFileName(dbname));
        }
        if (!s.IsOK()) {
            s = env->RemoveFile(tmp);
        }
        return s;
    }

}