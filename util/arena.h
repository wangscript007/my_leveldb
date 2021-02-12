//
// Created by kuiper on 2021/2/7.
//

#ifndef MY_LEVELDB_ARENA_H
#define MY_LEVELDB_ARENA_H

#include <atomic>
#include <cassert>
#include <cstdint>
#include <vector>

namespace leveldb {

    class Arena {
    public:
        Arena();

        Arena(const Arena &) = delete;
        Arena &operator=(const Arena &) = delete;

        ~Arena();

        char *Allocate(size_t bytes);

        char *AllocateAligned(size_t bytes);

        std::size_t MemoryUsage() const {
            return memory_usage_.load(std::memory_order_relaxed);
        }

    private:

        // @brief 当Allocate时当前块内存不够时就会走这个
        // @param bytes 申请的字节数
        // @return 申请内存的地址
        char *AllocateFallback(size_t bytes);

        // @brief 分配一个块
        // @param block_bytes
        // @return
        char *AllocateNewBlock(size_t block_bytes);

        char *alloc_ptr_;
        size_t alloc_bytes_remaining_;
        std::vector<char *> blocks_;
        std::atomic<size_t> memory_usage_;
    };

    inline char *Arena::Allocate(size_t bytes) {
        assert(bytes > 0);
        if (bytes <= alloc_bytes_remaining_) {
            char *result = alloc_ptr_;
            alloc_ptr_ += bytes;
            alloc_bytes_remaining_ -= bytes;
            return result;
        }
        return AllocateFallback(bytes);
    }
}


#endif //MY_LEVELDB_ARENA_H
