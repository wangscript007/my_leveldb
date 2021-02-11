//
// Created by kuiper on 2021/2/8.
//

#ifndef MY_LEVELDB_HASH_H
#define MY_LEVELDB_HASH_H

#include <cstddef>
#include <cstdint>

namespace leveldb {

    // @brief 对字符串进行hash得到一个32位的无符号整形.
    // @param data 需要hash的字符串首地址.
    // @param n 需要hash的字符串的长度.
    // @param seed 种子.
    uint32_t Hash(const char *data, size_t n, uint32_t seed);

}

#endif //MY_LEVELDB_HASH_H
