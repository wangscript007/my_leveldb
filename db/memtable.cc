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

    bool MemTable::Get(const LookupKey &lookup_key, std::string *value, Status *s) {
        Slice memtable_key = lookup_key.memtable_key();
        Table::Iterator iter(&table_);
        iter.Seek(memtable_key.data()); // seek到>= memtable_key的节点.
        if (!iter.Valid()) {
            return false;
        }

        // entry format:
        // keyLength:  varint32
        // userKey  :  char[keyLength - 8]
        // valueLen :  varint32
        // value    :   char[valueLen]
        const char *entry = iter.key();
        uint32_t key_length;

        // 读取keyLength
        const char *key_ptr = GetVarint32Ptr(entry, entry + 5, &key_length);
        Slice userKey = Slice(key_ptr, key_length - 8);
        if (comparator_.comparator.user_comparator()->Compare(userKey, lookup_key.user_key()) == 0) {
            const uint64_t seq_and_type = DecodeFixed64(key_ptr + key_length - 8);
            switch (static_cast<ValueType>(seq_and_type & 0xff)) {
                case kTypeValue: {
                    Slice val = GetLengthPrefixedSlice(key_ptr + key_length);
                    value->assign(val.data(), val.size());
                    *s = Status::OK(); // FIXME
                    return true;
                }
                case kTypeDeletion: {
                    *s = Status::NotFound(Slice());
                    return true;
                }
                default:
                    // Unreachable.
                    assert(false);
            }
        }

        return false;
    }
}

