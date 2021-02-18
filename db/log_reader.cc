//
// Created by kuiper on 2021/2/18.
//

#include "db/log_reader.h"
#include "leveldb/env.h"

namespace leveldb::log {

    Reader::Reader(SequentialFile *file, Reporter *reporter, bool checksum, uint64_t initial_offset)
            : file_(file),
              reporter_(reporter),
              checksum_(checksum),
              backing_store_(new char[kBlockSize]),
              buffer_(),
              eof_(false),
              last_record_offset_(0),
              end_of_buffer_offset_(0),
              initial_offset_(initial_offset),
              resyncing_(initial_offset > 0) {
    }

    Reader::~Reader() {
        delete[] backing_store_;
    }

    bool Reader::SkipToInitialBlock() {
        // initial_offset在当前block内的偏移.
        const size_t offset_in_block = initial_offset_ % kBlockSize;
        // initial_offset所在块的块起始地址
        uint64_t block_start_position = initial_offset_ - offset_in_block;

        // 如果block内的偏移在最后的6字节里, 说明存的不是数据，而是我们填充的\x00 那么跳转到下一个block
        if (offset_in_block > kBlockSize - 6) {
            block_start_position += kBlockSize;
        }

        end_of_buffer_offset_ = block_start_position;

        if (block_start_position > 0) {
            auto skip_status = file_->Skip(block_start_position);
            if (!skip_status.IsOK()) {
                ReportDrop(block_start_position, skip_status);
                return false;
            }
        }
        return true;
    }


    uint64_t Reader::LastRecordOffset() {
        return last_record_offset_;
    }

    void Reader::ReportCorruption(uint64_t bytes, const char *reason) {
        ReportDrop(bytes, Status::Corruption(reason));
    }

    void Reader::ReportDrop(uint64_t bytes, const Status &reason) {
        if (reporter_ != nullptr &&
            end_of_buffer_offset_ - buffer_.size() - bytes >= initial_offset_) {
            reporter_->Corruption(static_cast<size_t>(bytes), reason);
        }
    }

    bool Reader::ReadRecord(Slice *record, std::string *scratch) {
        if (last_record_offset_ < initial_offset_) {
            // 如果当前的偏移 < 指定的偏移 则需要seek
            if (!SkipToInitialBlock())
                return false;
        }
        scratch->clear();
        record->clear();

    }

    unsigned int Reader::ReadPhysicalRecord(Slice *result) {

    }

}