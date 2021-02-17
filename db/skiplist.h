//
// Created by kuiper on 2021/2/16.
//

#ifndef MY_LEVELDB_SKIPLIST_H
#define MY_LEVELDB_SKIPLIST_H

#include "util/arena.h"
#include "util/random.h"

namespace leveldb {

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

        // 可能返回nullptr
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
            assert(n >= 0);
            return next_[n].load(std::memory_order_acquire);
        }

        void SetNext(int n, Node *node) {
            assert(n >= 0);
            next_[n].store(node, std::memory_order_release);
        }

        Node *NoBarrier_Next(int n) {
            assert(n >= 0);
            return next_[n].load(std::memory_order_relaxed);
        }

        void NoBarrier_SetNext(int n, Node *node) {
            assert(n >= 0);
            next_[n].store(node, std::memory_order_relaxed);
        }

    public:
        Key const key;
    private:
        // 数组的长度=节点的高度 分别记录当前节点在高度为n的next
        std::atomic<Node *> next_[1];
    };

    template<typename Key, typename Compare>
    SkipList<Key, Compare>::SkipList(Compare cmp, Arena *arena)
            :   comparator_(cmp),
                arena_(arena),
                head_(NewNode(0, kMaxHeight)),
                max_height_(1),
                rnd_(0xdeadbeef) {
        for (int i = 0; i < kMaxHeight; ++i) {
            head_->SetNext(i, nullptr);
        }
    }

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

    template<typename Key, class Comparator>
    void SkipList<Key, Comparator>::Insert(const Key &key) {
        Node *prev[kMaxHeight];
        Node *x = FindGreaterOrEqual(key, prev);
        assert(x == nullptr || !Equal(key, x->key));

        // 随机加高SkipList
        int height = RandomHeight();
        if (height > GetMaxHeight()) {
            for (int i = GetMaxHeight(); i < height; ++i) {
                prev[i] = head_;
            }
            max_height_.store(height, std::memory_order_relaxed);
        }

        // Insert
        x = NewNode(key, height);
        for (int i = 0; i < height; ++i) {
            x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
            prev[i]->SetNext(i, x);
        }
    }

    template<typename Key, class Comparator>
    bool SkipList<Key, Comparator>::Contains(const Key &key) const {
        return false;
    }

    template<typename Key, class Comparator>
    bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key &key, SkipList::Node *n) const {
        return (n != nullptr) && (comparator_(n->key, key) < 0);
    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *SkipList<Key, Compare>::FindLessThan(const Key &key) const {

    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *
    SkipList<Key, Compare>::FindGreaterOrEqual(const Key &key, SkipList::Node **prev) const {
        Node *x = head_;
        int level = GetMaxHeight() - 1;
        for (;;) {
            Node *next = x->Next(level);
            if (KeyIsAfterNode(key, next)) {
                // 如果key在node之后，则跳转下一节点
                x = next;
            } else {
                // key在node之前
                if (prev != nullptr) prev[level] = x;
                if (level == 0) {
                    return next;
                } else {
                    level--;
                }
            }
        }
    }

    template<typename Key, typename Compare>
    typename SkipList<Key, Compare>::Node *SkipList<Key, Compare>::FindLast() const {
        Node *x = head_;
        int level = GetMaxHeight() - 1;
        for (;;) {
            Node *next = x->Next(level);
            if (next == nullptr) {
                if (level == 0) {
                    return x;
                } else {
                    level--;
                }
            } else {
                x = next;
            }
        }
    }


}

#endif //MY_LEVELDB_SKIPLIST_H
