//
// Created by kuiper on 2021/2/18.
//

#ifndef MY_LEVELDB_LOG_WRITER_H
#define MY_LEVELDB_LOG_WRITER_H

#include <cstdint>

#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

    class WritableFile;

    namespace log {

        class Writer {
        public:
            explicit Writer(WritableFile *dest);
            Writer(WritableFile *dest, uint64_t dest_length);

            Writer(const Writer&) = delete;
            Writer& operator=(const Writer&) = delete;

            ~Writer() = default;

            Status AddRecord(const Slice& slice);

        private:
            Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

            WritableFile *dest_;
            int block_offset_;  // 在块中的当前offset

            uint32_t type_crc_[kMaxRecordType + 1]{}; // crc码

        };


    }

}


#endif //MY_LEVELDB_LOG_WRITER_H
