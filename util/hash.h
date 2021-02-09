//
// Created by kuiper on 2021/2/8.
//

#ifndef MY_LEVELDB_HASH_H
#define MY_LEVELDB_HASH_H

#include <cstddef>
#include <cstdint>

namespace leveldb {

    uint32_t Hash(const char *data, size_t n, uint32_t seed);

}

#endif //MY_LEVELDB_HASH_H
