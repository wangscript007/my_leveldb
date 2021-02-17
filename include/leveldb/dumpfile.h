//
// Created by kuiper on 2021/2/16.
//

#ifndef MY_LEVELDB_DUMPFILE_H
#define MY_LEVELDB_DUMPFILE_H

#include <string>

#include "leveldb/env.h"
#include "leveldb/export.h"
#include "leveldb/status.h"

namespace leveldb {

    LEVELDB_EXPORT Status DumpFile(Env *env, const std::string &fname, WritableFile *dst);

}

#endif //MY_LEVELDB_DUMPFILE_H
