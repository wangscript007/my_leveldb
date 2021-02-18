//
// Created by kuiper on 2021/2/18.
//

#ifndef MY_LEVELDB_LOG_READER_H
#define MY_LEVELDB_LOG_READER_H

#include <cstdint>

#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"


namespace leveldb {

    class SequentialFile;

    namespace log {

        class Reader {
        public:

            // 用于汇报日志.
            class Reporter {
            public:
                virtual ~Reporter() = default;

                // @brief 检测到损坏
                // @param bytes大致损坏的字节数.
                // @param
                virtual void Corruption(size_t bytes, const Status &status) = 0;
            };


            Reader(SequentialFile *file, Reporter *reporter, bool checksum, uint64_t initial_offset);

            Reader(const Reader &) = delete;

            Reader &operator=(const Reader &) = delete;

            ~Reader();

            bool ReadRecord(Slice *slice, std::string *scratch);

            uint64_t LastRecordOffset() const;

        private:
            enum {
                kEof = kMaxRecordType + 1,
                kBadRecord = kMaxRecordType + 2
            };

            // @brief 调到initial_offset所在的块的快首地址
            bool SkipToInitialBlock();

            // @brief 读取一个物理块到result中
            unsigned int ReadPhysicalRecord(Slice *result);

            // @brief 汇报损坏.
            void ReportCorruption(uint64_t bytes, const char *reason);
            void ReportDrop(uint64_t bytes, const Status &reason);

        private:
            SequentialFile *const file_;
            Reporter *const reporter_;
            bool const checksum_;
            char *const backing_store_;
            Slice buffer_;
            bool eof_;

            uint64_t last_record_offset_;
            uint64_t end_of_buffer_offset_;
            uint64_t const initial_offset_;
            bool resyncing_;
        };

    }

}

#endif //MY_LEVELDB_LOG_READER_H
