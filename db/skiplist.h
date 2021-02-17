//
// Created by kuiper on 2021/2/16.
//

#ifndef MY_LEVELDB_SKIPLIST_H
#define MY_LEVELDB_SKIPLIST_H

#include "util/arena.h"
#include "util/random.h"

namespace leveldb {
    class Arena;

    template<typename Key, class Comparator>
    class SkipList {
    private:
        struct Node;
    public:
        explicit SkipList(Comparator cmp, Arena *arena);

        SkipList(const SkipList &) = delete;

        SkipList &operator=(const SkipList &) = delete;

        void Insert(const Key &key);

        bool Contains(const Key &key) const;

        class Iterator {
        public:
            explicit Iterator(const SkipList *list);

            bool Valid() const;

            const Key &key() const;

            void Next();

            void Prev();

            void Seek(const Key &target);

            void SeekToFirst();

            void SeekToLast();

        private:
            const SkipList *list_;
            Node *node_;
        };

    private:
        inline int GetMaxHeight() const { return max_height_.load(std::memory_order_relaxed); }

        int RandomHeight();

        Node *NewNode(const Key &key, int height);

        bool Equal(const Key &lhs, const Key &rhs) const { return (comparator_(lhs, rhs) == 0); }

        bool KeyIsAfterNode(const Key &key, Node *n) const;

        Node *FindGreaterOrEqual(const Key &key, Node **prev) const;

        Node *FindLessThan(const Key &key) const;

        Node *FindLast() const;

    private:
        enum {
            kMaxHeight = 12
        };
        Comparator const comparator_;
        Arena *const arena_;
        Node *const head_;
        Random rnd_;
        std::atomic_int max_height_;
    };

    template<typename Key, typename Compare>
    struct SkipList<Key, Compare>::Node {
        explicit Node(const Key &k) : key(k) {}

        Node *Next(int n) {
            assert(n > 0);
            return next_[n].load(std::memory_order_acquire);
        }

        void SetNext(int n, Node *node) {
            assert(n > 0);
            next_[n].store(node, std::memory_order_release);
        }

        Node *NoBarrier_Next(int n) {
            assert(n > 0);
            return next_[n].load(std::memory_order_relaxed);
        }

        void NoBarrier_SetNext(int n, Node *node) {
            assert(n > 0);
            next_[n].store(node, std::memory_order_relaxed);
        }

    public:
        Key const key;
    private:
        // 数组的长度=节点的高度
        std::atomic<Node *> next_[1];
    };

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *SkipList<Key, Compare>::NewNode(const Key &key, int height) {
        auto require_bytes = sizeof(Node) + sizeof(std::atomic<Node *>) * (height - 1);
        char *const addr = arena_->AllocateAligned(require_bytes);
        return new(addr) Node(key);
    }

    template<typename Key, typename Compare>
    int SkipList<Key, Compare>::RandomHeight() {
        // Increase height with probability 1 in kBranching
        static const unsigned int kBranching = 4;
        int height = 1;
        while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
            height++;
        }
        assert(height > 0);
        assert(height <= kMaxHeight);
        return height;
    }

    template<typename Key, typename Compare>
    bool SkipList<Key, Compare>::KeyIsAfterNode(const Key &key, Node *n) const {
        return (n != nullptr) && (comparator_(n->key, key) < 0);
    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *SkipList<Key, Compare>::FindLessThan(const Key &key) const {

    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *
    SkipList<Key, Compare>::FindGreaterOrEqual(const Key &key, Node **prev) const {

    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *SkipList<Key, Compare>::FindLast() const {

    }


}

#endif //MY_LEVELDB_SKIPLIST_H
