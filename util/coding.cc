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

    void PutFixed64(std::string *dst, uint64_t value) {
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
        PutVarint32(dst, value.size());          // 写入varint编码的长度
        dst->append(value.data(), value.size()); // 写入数据
    }

    int VarintLength(uint64_t v) {
        int len = 1;
        while (v >= 128) {
            v >>= 7;
            len++;
        }
        return len;
    }


    bool GetVarint32(Slice *input, uint32_t *value) {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = GetVarint32Ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            *input = Slice(q, limit - q);
            return true;
        }
    }

    bool GetVarint64(Slice *input, uint64_t *value) {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = GetVarint64Ptr(p, limit, value);
        if (q == nullptr) {
            return false;
        } else {
            *input = Slice(q, limit - q);
            return true;
        }
    }

    bool GetLengthPrefixedSlice(Slice *input, Slice *result) {
        uint32_t len;
        if (GetVarint32(input, &len) && input->size() >= len) {
            // GetVarint32 eat 4 byte len
            *result = Slice(input->data(), len);
            input->remove_prefix(len);
            return true;
        } else {
            return false;
        }
    }

    const char *GetVarint32PtrFallback(const char *p, const char *limit, uint32_t *value) {
        // FIXME
        return nullptr;
    }

    const char *GetVarint32Ptr(const char *p, const char *limit, uint32_t *value) {
        if (p < limit) {
            uint32_t result = *(reinterpret_cast<const uint8_t *>(p));
            if ((result & 128) == 0) {
                *value = result;
                return p + 1;
            }
        }
        return GetVarint32PtrFallback(p, limit, value);
    }

    const char *GetVarint64Ptr(const char *p, const char *limit, uint64_t *value) {

        // FIXME
        return nullptr;
    }

}

