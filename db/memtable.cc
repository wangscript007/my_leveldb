//
// Created by kuiper on 2021/2/16.
//

#include "db/memtable.h"
#include "db/dbformat.h"
#include "leveldb/status.h"

namespace leveldb {

    // data是一个varint_len + data
    static Slice GetLengthPrefixedSlice(const char *data) {
        uint32_t len;
        const char *p = data;
        p = GetVarint32Ptr(p, p + 5, &len);
        return Slice(p, len);
    }

    static const char *EncodeKey(std::string *scratch, const Slice &target) {
        scratch->clear();
        PutVarint32(scratch, target.size());
        scratch->append(target.data(), target.size());
        return scratch->data();
    }


    MemTable::MemTable(const InternalKeyComparator &comparator)
            : comparator_(comparator), refs_(0), table_(KeyComparator(comparator), &arena_) {}

    MemTable::~MemTable() {
        assert(refs_ == 0);
    }

    size_t MemTable::ApproximateMemoryUsage() {
        return arena_.MemoryUsage();
    }

    int MemTable::KeyComparator::operator()(const char *aptr, const char *bptr) const {
        Slice a = GetLengthPrefixedSlice(aptr);
        Slice b = GetLengthPrefixedSlice(bptr);
        return this->comparator.Compare(a, b);
    }

    // MemTableIterator is a Wrapper of SkipList::Iterator
    class MemTableIterator : public Iterator {
    public:
        explicit MemTableIterator(MemTable::Table *table)
                : iter_(table) {}

        MemTableIterator(const MemTableIterator &) = delete;

        MemTableIterator &operator=(const MemTableIterator &) = delete;

        ~MemTableIterator() override = default;

        bool Valid() const override { return iter_.Valid(); }

        void Seek(const Slice &k) override {
            iter_.Seek(EncodeKey(&tmp_, k));
        }

        void SeekToFirst() override {
            iter_.SeekToFirst();
        }

        void SeekToLast() override {
            iter_.SeekToLast();
        }

        void Next() override {
            iter_.Next();
        }

        void Prev() override {
            iter_.Prev();
        }

        Slice Key() const override {
            return GetLengthPrefixedSlice(iter_.key());
        }

        Slice Value() const override {
            Slice key_slice = GetLengthPrefixedSlice(iter_.key());
            return GetLengthPrefixedSlice(key_slice.data() + key_slice.size());
        }

        Status status() const override {
            return Status::OK();
        }

    private:
        MemTable::Table::Iterator iter_;
        std::string tmp_;   // For encode use.
    };

    Iterator *MemTable::NewIterator() {
        return new MemTableIterator(&table_);
    }

    void MemTable::Add(SequenceNumber seq, ValueType type, const Slice &key, const Slice &value) {
        const size_t key_size = key.size();
        const size_t value_size = value.size();

        // varint32|key|varint32|value
        ParsedInternalKey parsed_internal_ley(key, seq, type);
        size_t internal_key_size = InternalKeyEncodingLength(parsed_internal_ley);

        const size_t total_size =
                VarintLength(internal_key_size) + internal_key_size + VarintLength(value_size) + value_size;
        char *buf = arena_.Allocate(total_size);

        // 写入varint32的internalKey长度
        char *pCur = EncodeVarint32(buf, internal_key_size);

        // 写入internalKey
        InternalKey internal_key;
        internal_key.SetFrom(parsed_internal_ley);
        Slice encoded_key_slice = internal_key.Encode();
        std::memcpy(pCur, encoded_key_slice.data(), encoded_key_slice.size());
        pCur += encoded_key_slice.size();

        // 写入varint64的value长度
        pCur = EncodeVarint32(pCur, value_size);

        // 写入value
        std::memcpy(pCur, value.data(), value_size);

        assert(pCur + value_size == buf + total_size);
        table_.Insert(buf);
    }

    bool MemTable::Get(const LookupKey &key, std::string *value, Status *s) {
        return false;
    }
}

