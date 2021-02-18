//
// Created by kuiper on 2021/2/18.
//

#include "db/log_reader.h"

namespace leveldb {
    namespace log {

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
            return false;
        }

        bool Reader::ReadRecord(Slice *slice, std::string *scratch) {

        }

        uint64_t Reader::LastRecordOffset() {
            return last_record_offset_;
        }

        void Reader::ReportCorruption(uint64_t bytes, const char *reason) {

        }

        void Reader::ReportDrop(uint64_t bytes, const Status &reason) {

        }

        unsigned int Reader::ReadPhysicalRecord(Slice *result) {

        }

    }
}