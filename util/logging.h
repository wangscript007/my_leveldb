//
// Created by kuiper on 2021/2/12.
//

#ifndef MY_LEVELDB_LOGGING_H
#define MY_LEVELDB_LOGGING_H

#include <string>

namespace leveldb {

    class Slice;

    class WritableFile;

    // @brief 将一个数字已人类可读的格式追加到字符串末尾.
    // @param str 字符串指针.
    // @param num 数字.
    void AppendNumberTo(std::string *str, uint64_t num);

    //
    //
    //
    void AppendEscapedStringTo(std::string *str, const Slice &value);

    // @brief 将一个数字转为人类可读格式的字符串.
    // @param 需要转换的数字.
    // @return 转换后的字符串.
    std::string NumberToString(uint64_t num);

    //
    //
    std::string EscapeString(const Slice &value);

    //
    //
    //
    bool ConsumeDecimalNumber(Slice *in, uint64_t *val);

}


#endif //MY_LEVELDB_LOGGING_H
