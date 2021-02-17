//
// Created by kuiper on 2021/2/17.
//

#include "filename.h"

namespace leveldb {

    static std::string MakeFileName(const std::string &dbname, uint64_t number, const char* suffix) {
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

    bool ParseFileName(const std::string &filename, uint64_t *number, FileType *type) {

    }

    Status SetCurrentFile(Env *env, const std::string &dbname, uint64_t descriptor_number) {

    }

}