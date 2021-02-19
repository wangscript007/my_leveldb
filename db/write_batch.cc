//
// Created by kuiper on 2021/2/12.
//

#include "leveldb/write_batch.h"
#include "db/write_batch_internal.h"
#include "db/dbformat.h"
#include "db/memtable.h"
#include "util/coding.h"


namespace leveldb {

    // WriteBatch的header= 8字节的sequence_nbr + 4 字节的count
    static const size_t kHeader = 12;

    WriteBatch::WriteBatch() {
        Clear();
    }

    WriteBatch::~WriteBatch() = default;

    WriteBatch::Handler::~Handler() = default;

    void WriteBatch::Clear() {
        rep_.clear();
        rep_.resize(kHeader);
    }

    size_t WriteBatch::ApproximateSize() const {
        return rep_.size();
    }

    Status WriteBatch::Iterate(Handler *handler) const {
        Slice input(rep_);
        if (input.size() < kHeader) {
            return Status::Corruption("malformed WriteBatch(too small)");
        }

        input.remove_prefix(kHeader);
        Slice key, value;
        int found = 0;
        while (!input.empty()) {
            found++;
            char tag = input[0];
            input.remove_prefix(1);
            switch (static_cast<ValueType>(tag)) {
                case kTypeValue:
                    if (GetLengthPrefixedSlice(&input, &key) && GetLengthPrefixedSlice(&input, &value)) {
                        handler->Put(key, value);
                    } else {
                        return Status::Corruption("bad WriteBatch Put");
                    }
                    break;
                case kTypeDeletion:
                    if (GetLengthPrefixedSlice(&input, &key)) {
                        handler->Delete(key);
                    } else {
                        return Status::Corruption("bad WriteBatch Delete");
                    }
                    break;
                default:
                    return Status::Corruption("unknown WriteBatch tag");
            }
            if (found != WriteBatchInternal::Count(this)) {
                return Status::Corruption("WriteBatch has wrong count");
            } else {
                return Status::OK();
            }
        }

    }

    void WriteBatch::Put(const Slice &key, const Slice &value) {
        WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
        rep_.push_back(static_cast<char>(kTypeValue));
        PutLengthPrefixedSlice(&rep_, key);
        PutLengthPrefixedSlice(&rep_, value);
    }

    void WriteBatch::Delete(const Slice &key) {
        WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
        rep_.push_back(static_cast<char>(kTypeDeletion));
        PutLengthPrefixedSlice(&rep_, key);
    }

    void WriteBatch::Append(const WriteBatch &source) {
        WriteBatchInternal::Append(this, &source);
    }

    class MemTableInserter : public WriteBatch::Handler {
    public:
        SequenceNumber sequence_;
        MemTable *mem_;

        void Put(const Slice &key, const Slice &val) override {
            mem_->Add(sequence_, kTypeValue, key, val);
            sequence_++;
        }

        void Delete(const Slice &key) override {
            mem_->Add(sequence_, kTypeDeletion, key, Slice());
            sequence_++;
        }
    };

    // WriteBatchInternal start.
    int WriteBatchInternal::Count(const WriteBatch *batch) {
        return DecodeFixed32(batch->rep_.data() + 8);
    }

    void WriteBatchInternal::SetCount(WriteBatch *batch, int n) {
        EncodeFixed32(batch->rep_.data() + 8, n);
    }

    SequenceNumber WriteBatchInternal::Sequence(const WriteBatch *batch) {
        return SequenceNumber(DecodeFixed64(batch->rep_.data()));
    }

    void WriteBatchInternal::SetSequence(WriteBatch *batch, SequenceNumber seq) {
        EncodeFixed64(batch->rep_.data(), seq);
    }

    void WriteBatchInternal::SetContents(WriteBatch *batch, const Slice &contents) {
        assert(contents.size() >= kHeader);
        batch->rep_.assign(contents.data(), contents.size());
    }

    Status WriteBatchInternal::InsertInto(const WriteBatch *batch, MemTable *memTable) {
        MemTableInserter inserter;
        inserter.sequence_ = WriteBatchInternal::Sequence(batch);
        inserter.mem_ = memTable;
        return batch->Iterate(&inserter);
    }

    void WriteBatchInternal::Append(WriteBatch *dst, const WriteBatch *src) {
        SetCount(dst, Count(dst) + Count(src));
        assert(src->rep_.size() > kHeader);
        dst->rep_.append(src->rep_.data() + kHeader, src->rep_.size() - kHeader);
    }

}