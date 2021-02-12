//
// Created by kuiper on 2021/2/4.
//

#ifndef MY_LEVELDB_SLICE_H
#define MY_LEVELDB_SLICE_H

#include <cstddef>
#include <string>
#include <cstring>
#include "leveldb/export.h"
#include "leveldb/cxx.h"

namespace leveldb {

    // likes std::string_view.
    class LEVELDB_EXPORT Slice {
    public:
        Slice() : data_(""), size_(0) {}

        Slice(const char *d, size_t n) : data_(d), size_(n) {}

        Slice(const std::string &s) : data_(s.data()), size_(s.size()) {}

        Slice(const char *s) : data_(s), size_(strlen(s)) {}

        // 显示支持拷贝构造
        Slice(const Slice &) = default;

        // 显示支持拷贝赋值
        Slice &operator=(const Slice &) = default;

        NO_DISCARD
        const char *data() const { return data_; }

        NO_DISCARD
        size_t size() const { return size_; }

        NO_DISCARD
        bool empty() const { return size_ == 0; }

        char operator[](size_t n) const {
            assert(n < size());
            return data_[n];
        }

        void clear() {
            data_ = "";
            size_ = 0;
        }

        void remove_prefix(size_t n) {
            assert(n <= size());
            data_ += 1;
            size_ -= 1;
        }

        NO_DISCARD
        bool starts_with(const Slice &x) const {
            return size_ >= x.size() && std::memcmp(data_, x.data(), x.size()) == 0;
        }

        NO_DISCARD
        std::string ToString() const {
            return std::string(data_, size_);
        }

        NO_DISCARD
        int compare(const Slice &b) const;

    private:
        const char *data_;
        std::size_t size_;
    };

    inline bool operator==(const Slice &x, const Slice &y) {
        return x.size() == y.size() && std::memcmp(x.data(), y.data(), x.size()) == 0;
    }

    inline bool operator!=(const Slice &x, const Slice &y) {
        return !operator==(x, y);
    }

    inline int Slice::compare(const Slice &b) const {
        const size_t min_size = size_ > b.size() ? b.size() : size_;
        int r = std::memcmp(data_, b.data_, min_size);
        if (r == 0) {
            if (size_ < b.size_) {
                r = -1;
            } else if (size_ > b.size_) {
                r = 1;
            }
        }
        return r;
    }


}

#endif //MY_LEVELDB_SLICE_H
