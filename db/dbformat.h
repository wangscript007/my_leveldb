//
// Created by kuiper on 2021/2/18.
//

#ifndef MY_LEVELDB_DBFORMAT_H
#define MY_LEVELDB_DBFORMAT_H

#include <cstdint>

#include "leveldb/slice.h"
#include "leveldb/comparator.h"
#include "util/coding.h"
#include "util/logging.h"
#include "port/port.h"

namespace leveldb {
    namespace config {

        static const int kNumLevels = 7;

        // 当有这么多文件时, level0的压缩开始
        static constexpr int kL0_CompactionTrigger = 4;

        // 到达这个点后 level0的下沉开始
        static constexpr int kL0_SlowdownWritesTrigger = 8;

        // level0的最大文件数目. 到达这个点停止写入
        static constexpr int kL0_StopWritesTrigger = 12;

        static constexpr int kMaxMemCompactLevel = 2;

        static constexpr int kReadBytesPeriod = 1048576;

    }   // namespace leveldb::config

    class InternalKey;

    enum ValueType {
        kTypeDeletion = 0x0, kTypeValue = 0x1
    };

    static const ValueType kValueTypeForSeek = kTypeValue;

    typedef uint64_t SequenceNumber;

    // 空出8位, type和序列号共同存储在一个64bits里
    static constexpr SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);

    struct ParsedInternalKey {
        Slice user_key;
        SequenceNumber sequence;
        ValueType type;

        ParsedInternalKey() {}

        ParsedInternalKey(const Slice &u, const SequenceNumber &seq, ValueType t)
                : user_key(u), sequence(seq), type(t) {}

        std::string DebugString() const;
    };

    // 返回key编码需要的长度
    __STATIC_INLINE__
    size_t InternalKeyEncodingLength(const ParsedInternalKey &key) {
        return key.user_key.size() + 8;
    }

    __STATIC_INLINE__
    uint64_t PackSequenceAndType(uint64_t seq, ValueType t) {
        assert(seq <= kMaxSequenceNumber);
        assert(t <= kValueTypeForSeek);
        return (seq << 8) | (static_cast<uint64_t>(t));
    }

    // 将key序列化后存储到dest中
    __STATIC_INLINE__
    void AppendInternalKey(std::string *dest, const ParsedInternalKey &key) {
        // dest->clear(); should do this ???
        dest->append(key.user_key.ToString());
        PutFixed64(dest, PackSequenceAndType(key.sequence, key.type));
    }

    // 尝试解析src_internal_key
    // 如果解析成功, 解析后的数据存放在result中并返回true
    // 如果解析失败, 返回false.
    __STATIC_INLINE__
    bool ParseInternalKey(const Slice &src_internal_key, ParsedInternalKey *result) {
        const size_t n = src_internal_key.size();
        if (n < 8) {
            return false;
        }
        uint64_t seq_and_type = DecodeFixed64(src_internal_key.data() + n - 8);
        uint8_t c = seq_and_type & 0xff;

        result->sequence = seq_and_type >> 8;
        result->type = static_cast<ValueType>(c);
        result->user_key = Slice(src_internal_key.data(), n - 8);

        return c <= static_cast<uint8_t>(kTypeValue);
    }

    // 提取user_key部分
    // internal_key是序列化后的数据, 包含了user_key和 seq+type(8bytes)
    __STATIC_INLINE__
    Slice ExtractUserKey(const Slice &internal_key) {
        assert(internal_key.size() >= 8);
        return Slice(internal_key.data(), internal_key.size() - 8);
    }



    // FIXME InternalFilterPolicy


    class InternalKey {
    private:
        std::string rep_;
    public:
        InternalKey() {}

        InternalKey(const Slice &user_key, SequenceNumber s, ValueType t) {
            AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
        }

        bool DecodeFrom(const Slice &s) {
            rep_.assign(s.data(), s.size());
            return !rep_.empty();
        }

        Slice Encode() const {
            assert(false == rep_.empty());
            return rep_;
        }

        Slice user_key() const {
            return ExtractUserKey(rep_);
        }

        void SetFrom(const ParsedInternalKey &pik) {
            rep_.clear();
            AppendInternalKey(&rep_, pik);
        }

        void Clear() noexcept {
            rep_.clear();
        }

        std::string DebugString() const;
    };

    class InternalKeyComparator : public Comparator {
    private:
        const Comparator *user_comparator_;
    public:
        explicit InternalKeyComparator(const Comparator *c) : user_comparator_(c) {}

        const char *Name() const override {
            return "leveldb.InternalKeyComparator";
        }

        int Compare(const Slice &a, const Slice &b) const override;

        void FindShortSuccessor(std::string *key) const override;

        void FindShortestSeparator(std::string *start, const Slice &limit) const override;

        const Comparator *user_comparator() const { return user_comparator_; }

        int Compare(const InternalKey &a, const InternalKey &b) const {
            return Compare(a.Encode(), b.Encode());
        }
    };

    // Helper-Class 用于DBImpl::Get()
    class LookupKey {
    public:
    private:
    };

}

#endif //MY_LEVELDB_DBFORMAT_H
