//
// Created by kuiper on 2021/2/12.
//

#include "leveldb/write_batch.h"
#include "db/write_batch_internal.h"

#include "util/coding.h"

namespace leveldb {

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

    }

}