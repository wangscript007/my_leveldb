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
        uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);

    }

    void PutVarint64(std::string *dst, uint32_t value) {

    }

    void PutLengthPrefixedSlice(std::string *dst, const Slice &value) {

    }

}

