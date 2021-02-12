//
// Created by kuiper on 2021/2/12.
//

#include "util/logging.h"
#include "leveldb/slice.h"

namespace leveldb {

    void AppendNumberTo(std::string *str, uint64_t num) {
        (*str) += std::to_string(num);
    }

    void AppendEscapedStringTo(std::string *str, const Slice &value) {
        for (size_t i = 0; i < value.size(); i++) {
            char c = value[i];
            if (c >= ' ' && c <= '~') {
                str->push_back(c);
            } else {
                char buf[10];
                std::snprintf(buf, sizeof(buf), "\\x%02x",
                              static_cast<unsigned int>(c) & 0xff);
                str->append(buf);
            }
        }
    }

    std::string NumberToString(uint64_t num) {
        return std::to_string(num);
    }

    std::string EscapeString(const Slice &value) {
        std::string res;
        AppendEscapedStringTo(&res, value);
        return res;
    }

    bool ConsumeDecimalNumber(Slice *in, uint64_t *val) {

    }

}