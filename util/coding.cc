//
// Created by kuiper on 2021/2/8.
//

#include "util/coding.h"


namespace leveldb {


    void PutFixed32(std::string *dst, uint32_t value) {
        char buf[sizeof(value)];
        EncodeFixed32(buf, value);
        dst->append(buf, sizeof(buf));
    }

    void PutFixed64(std::string *dst, uint32_t value) {
        char buf[sizeof(value)];
        EncodeFixed64(buf, value);
        dst->append(buf, sizeof(buf));
    }

    void PutVarint32(std::string *dst, uint32_t value) {
        char buf[5];
        char *ptr = EncodeVarint32(buf, value);
        dst->append(buf, ptr - buf);
    }

    void PutVarint64(std::string *dst, uint32_t value) {
        char buf[10];
        char *ptr = EncodeVarint64(buf, value);
        dst->append(buf, ptr - buf);
    }

    void PutLengthPrefixedSlice(std::string *dst, const Slice &value) {
        // 先写入长度
        PutVarint32(dst, value.size());
        // 在写入字符串内容
        dst->append(value.data(), value.size());
    }


    bool GetVarint32(Slice *input, uint32_t *value) {
        return false;
    }

    bool GetVarint64(Slice *input, uint64_t *value) {
        return false;
    }

    bool GetLengthPrefixedSlice(Slice *input, Slice *result) {
        return false;
    }


    int VarintLength(uint64_t v) {
        int len = 1;
        while (v >= 128) {
            v >>= 7;
            len++;
        }
        return len;
    }

}

