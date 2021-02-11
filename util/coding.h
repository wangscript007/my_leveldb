//
// Created by kuiper on 2021/2/8.
//

#ifndef MY_LEVELDB_CODING_H
#define MY_LEVELDB_CODING_H

#include <cstdint>
#include <cstring>
#include <string>
#include <variant>

#include "leveldb/slice.h"
#include "port/port.h"

namespace leveldb {

    void PutFixed32(std::string *dst, uint32_t value);
    void PutFixed64(std::string *dst, uint32_t value);
    void PutVarint32(std::string *dst, uint32_t value);
    void PutVarint64(std::string *dst, uint32_t value);
    void PutLengthPrefixedSlice(std::string *dst, const Slice &value);

    bool GetVarint32(Slice *input, uint32_t *value);
    bool GetVarint64(Slice *input, uint64_t *value);
    bool GetLengthPrefixedSlice(Slice *input, Slice *result);
    int VarintLength(uint64_t v);

    const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t *value);
    const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t *value);
    const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t *value);

    // 将一个无符号32位整形编码成varint
    static inline char *EncodeVarint32(char *dst, uint32_t v) {
        auto *ptr = reinterpret_cast<uint8_t *>(dst);
        static const int B = 128; // 1000 0000
        // 0000 0001
        if (v < (1U << 7U)) {             // < 0000 0000 1000 0000 = 128
            *(ptr++) = v;
        } else if (v < (1U << 14U)) {     // < 0100 0000 0000 0000 = 16384
            *(ptr++) = v | B;
            *(ptr++) = v >> 7;
        } else if (v < (1U << 21U)) {     // < 0001 0000 0000 0000 0000 0000
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = v >> 14;
        } else if (v < (1U << 28U)) {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = v >> 21;
        } else {
            *(ptr++) = v | B;
            *(ptr++) = (v >> 7) | B;
            *(ptr++) = (v >> 14) | B;
            *(ptr++) = (v >> 21) | B;
            *(ptr++) = v >> 28;
        }
        return reinterpret_cast<char *>(ptr);
    }

    static inline char *EncodeVarint64(char *dst, uint64_t v) {
        static constexpr int B = 128;
        auto *ptr = reinterpret_cast<uint8_t *>(dst);
        while (v > B) {
            *(ptr++) = v | B;
            v >>= 7;
        }
        *(ptr++) = static_cast<uint8_t>(v);
        return reinterpret_cast<char *>(ptr);
    }

    static inline void EncodeFixed32(char *dst, uint32_t value) {
        auto *const buffer = reinterpret_cast<uint8_t *>(dst);
        buffer[0] = static_cast<uint8_t>(value);
        buffer[1] = static_cast<uint8_t>(value >> 8U);
        buffer[2] = static_cast<uint8_t>(value >> 16U);
        buffer[3] = static_cast<uint8_t>(value >> 24U);
    }

    static inline uint32_t DecodeFixed32(const char *ptr) {
        const auto *const buffer = reinterpret_cast<const uint8_t *>(ptr);
        return (static_cast<uint32_t>(buffer[0])) |
               (static_cast<uint32_t>(buffer[1]) << 8U) |
               (static_cast<uint32_t>(buffer[2]) << 16U) |
               (static_cast<uint32_t>(buffer[3]) << 24U);
    }

    static inline void EncodeFixed64(char *dst, uint64_t value) {
        auto *const buffer = reinterpret_cast<uint8_t *>(dst);
        buffer[0] = static_cast<uint8_t>(value);
        buffer[1] = static_cast<uint8_t>(value >> 8U);
        buffer[2] = static_cast<uint8_t>(value >> 16U);
        buffer[3] = static_cast<uint8_t>(value >> 24U);
        buffer[4] = static_cast<uint8_t>(value >> 32U);
        buffer[5] = static_cast<uint8_t>(value >> 40U);
        buffer[6] = static_cast<uint8_t>(value >> 48U);
        buffer[7] = static_cast<uint8_t>(value >> 56U);
    }

    static inline uint64_t DecodeFixed64(const char *ptr) {
        const auto *const buffer = reinterpret_cast<const uint8_t *>(ptr);
        return (static_cast<uint64_t>(buffer[0])) |
               (static_cast<uint64_t>(buffer[1]) << 8U) |
               (static_cast<uint64_t>(buffer[2]) << 16U) |
               (static_cast<uint64_t>(buffer[3]) << 24U) |
               (static_cast<uint64_t>(buffer[4]) << 32U) |
               (static_cast<uint64_t>(buffer[5]) << 40U) |
               (static_cast<uint64_t>(buffer[6]) << 48U) |
               (static_cast<uint64_t>(buffer[7]) << 56U);
    }


}


#endif //MY_LEVELDB_CODING_H
