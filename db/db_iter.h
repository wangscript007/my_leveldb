//
// Created by kuiper on 2021/2/20.
//

#ifndef MY_LEVELDB_DB_ITER_H
#define MY_LEVELDB_DB_ITER_H

#include <cstdint>
#include "db/dbformat.h"
#include "leveldb/db.h"

namespace leveldb {

    class DBImpl;

    Iterator *NewDBIterator(DBImpl *db,
                            const Comparator *user_key_comparator,
                            Iterator *internal_iter,
                            SequenceNumber sequence,
                            uint32_t seed);


}

#endif //MY_LEVELDB_DB_ITER_H
