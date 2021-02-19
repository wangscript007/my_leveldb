//
// Created by kuiper on 2021/2/19.
//

#ifndef MY_LEVELDB_SNAPSHOT_H
#define MY_LEVELDB_SNAPSHOT_H

#include "db/dbformat.h"
#include "leveldb/db.h"

namespace leveldb {
    class SnapshotList;

    class SnapshotImpl : public Snapshot {
    public:
        SnapshotImpl(SequenceNumber sequence_number)
                : sequence_number_(sequence_number) {}

        SequenceNumber sequence_number() const {
            return sequence_number_;
        }

    private:
        friend class SnapshotList;

        // 双向循环链表
        SnapshotImpl *prev_;
        SnapshotImpl *next_;

        const SequenceNumber sequence_number_;

#if !defined(NDEBUG)
        SnapshotList *list_ = nullptr;
#endif
    };


    class SnapshotList {
    public:
        SnapshotList() : head_(0) {
            head_.next_ = &head_;
            head_.prev_ = &head_;
        }

        bool empty() const { return head_.next_ == &head_; }

        SnapshotImpl *oldest() const {
            assert(empty() == false);
            return head_.next_;
        }

        SnapshotImpl *newest() const {
            assert(empty() == false);
            return head_.prev_;
        }

        // 创建一个新的快照并且追加到链表的末尾
        SnapshotImpl *New(SequenceNumber sequence_number) {
            assert(empty() || sequence_number >= newest()->sequence_number_);

            SnapshotImpl *snapshot = new SnapshotImpl(sequence_number);
#if !defined(NDEBUG)
            snapshot->list_ = this;
#endif
            snapshot->next_ = &head_;
            snapshot->prev_ = head_.prev_;
            snapshot->prev_->next_ = snapshot;
            snapshot->next_->prev_ = snapshot;
            return snapshot;
        }

        void Delete(const SnapshotImpl *snapshot) {
#if !defined(NDEBUG)
            assert(snapshot->list_ == this);
#endif  // !defined(NDEBUG)
            snapshot->prev_->next_ = snapshot->next_;
            snapshot->next_->prev_ = snapshot->prev_;
            delete snapshot;
        }

    private:
        // dummy head of double-linked list of snapshots
        SnapshotImpl head_;
    };
}

#endif //MY_LEVELDB_SNAPSHOT_H
