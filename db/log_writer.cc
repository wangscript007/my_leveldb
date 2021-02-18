//
// Created by kuiper on 2021/2/18.
//

#include "db/log_writer.h"
#include "leveldb/env.h"
#include "port/port.h"
#include "util/coding.h"
#include "util/crc32c.h"

namespace leveldb::log {

    static void InitTypeCrc(uint32_t *type_crc) {
        // fixme.
    }

    Writer::Writer(WritableFile *dest) : dest_(dest), block_offset_(0) {
        InitTypeCrc(type_crc_);
    }

    Writer::Writer(WritableFile *dest, uint64_t dest_length)
            : dest_(dest), block_offset_(dest_length % kBlockSize) {
        InitTypeCrc(type_crc_);
    }

    Writer::~Writer() = default;

    Status Writer::AddRecord(const Slice &slice) {
        const char *ptr = slice.data();
        size_t left = slice.size();

        Status s;
        bool isBegin = true;
        do {
            // leftover : 当前块剩余可写入数.
            const int leftover = kBlockSize - block_offset_;
            assert(leftover >= 0);

            // 如果当前block不足于写入一个Header,则全部补满0
            if (leftover < kHeaderSize) {
                if (leftover > 0) {
                    static_assert(kHeaderSize == 7, "kHeaderSize of log file <> 7");
                    // <7 最多6个
                    dest_->Append(Slice("\x00\x00\x00\x00\x00\x00", leftover));
                }
                block_offset_ = 0;
            }

            assert(kBlockSize - block_offset_ - kHeaderSize >= 0);

            // avail: 当前块扣除header后还可以写入的空间.
            // avail和fragment_len都可能为0
            const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
            const size_t fragment_len = left < avail ? left : avail;

            RecordType type;
            const bool isEnd = (left == fragment_len);

            if (isBegin && isEnd) {
                type = kFullType;
            } else if (isBegin) {
                type = kFirstType;
            } else if (isEnd) {
                type = kLastType;
            } else {
                type = kMiddleType;
            }

            s = EmitPhysicalRecord(type, ptr, fragment_len);
            ptr += fragment_len;
            left -= fragment_len;
            isBegin = false;

        } while (s.IsOK() && left > 0);
        return s;
    }

    Status Writer::EmitPhysicalRecord(RecordType type, const char *ptr, size_t length) {
        assert(length <= 0xffff);  // 2 bytes max
        assert(block_offset_ + kHeaderSize + length <= kBlockSize);

        char buf[kHeaderSize];
        buf[4] = static_cast<char>(length & 0xffull);
        buf[5] = static_cast<char>(length >> 8ull);

        // 计算并写入CRC32C
        uint32_t crc = crc32c::Extend(type_crc_[type], ptr, length);
        crc = crc32c::Mask(crc);
        EncodeFixed32(buf, crc);

        // 写入物理块
        Status s = dest_->Append(Slice(buf, kHeaderSize));
        if (LIKELY(s.IsOK())) {
            s = dest_->Append(Slice(ptr, length));
            if (LIKELY(s.IsOK())) {
                s = dest_->Flush();
            }
        }

        // 如果写入s.IsNotOK(),也直接累计offset跳过这个段
        block_offset_ += static_cast<int>((kHeaderSize + length));
        return s;
    }

}