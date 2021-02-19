//
// Created by kuiper on 2021/2/19.
//

#ifndef MY_LEVELDB_WRITE_BATCH_INTERNAL_H
#define MY_LEVELDB_WRITE_BATCH_INTERNAL_H

#include "db/dbformat.h"
#include "leveldb/write_batch.h"

namespace leveldb {
    class MemTable;

    //  WriteBatchInternal 提供一些static方法用于操作WriteBatch
    //  这些方法我们不希望在WriteBatch中暴露出去.
    class WriteBatchInternal {
    public:

        // 返回batch中的条目数.
        static int Count(const WriteBatch *batch);

        // 给batch设置条目数量
        static void SetCount(WriteBatch *batch, int n);

        // 返回这个batch中的起始条目的序号
        static SequenceNumber Sequence(const WriteBatch *batch);

        // 将seq这个指定的seqNbr作为这个batch的起始条目的序列号.
        static void SetSequence(WriteBatch *batch, SequenceNumber seq);

        static Slice Contents(const WriteBatch *batch) { return Slice(batch->rep_); }

        static void SetContents(WriteBatch *batch, const Slice &contents);

        static size_t ByteSize(const WriteBatch *batch) { return batch->rep_.size(); }

        static Status InsertInto(const WriteBatch *batch, MemTable *memTable);

        static void Append(WriteBatch *dst, const WriteBatch *src);

    };
}

#endif //MY_LEVELDB_WRITE_BATCH_INTERNAL_H
