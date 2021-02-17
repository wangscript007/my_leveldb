//
// Created by kuiper on 2021/2/17.
//

#ifndef MY_LEVELDB_LOG_FORMAT_H
#define MY_LEVELDB_LOG_FORMAT_H

namespace leveldb {
    namespace log {

        enum RecordType {
            // Zero保留类型 用于预分配文件.
            kZeroType = 0,

            kFullType = 1,

            // 碎片段
            kFirstType = 2,
            kMiddleType = 3,
            kLastType = 4
        };

        static const int kMaxRecordType = kLastType;
        static const int kBlockSize = 32768;

        // 4 bytes checksum + 2 bytes length + 1 bytes for type.
        static const int kHeaderSize = 4 + 2 + 1;

    }
}

#endif //MY_LEVELDB_LOG_FORMAT_H
