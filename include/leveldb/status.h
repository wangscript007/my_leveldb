//
// Created by kuiper on 2021/2/9.
//

#ifndef MY_LEVELDB_STATUS_H
#define MY_LEVELDB_STATUS_H

#include <algorithm>
#include <string>

#include "leveldb/slice.h"
#include "leveldb/export.h"

namespace leveldb {

    class LEVELDB_EXPORT Status {
    public:
        Status() noexcept
                : state_(nullptr) {}

        ~Status() {
            delete[] state_;
        }

        Status(const Status &rhs);

        Status &operator=(const Status &rhs);

        Status(Status &&rhs) noexcept;

        Status &operator=(Status &&rhs) noexcept;

        NO_DISCARD
        bool IsOK() const { return state_ == nullptr; }

        NO_DISCARD
        bool IsNotFound() const { return code() == kNotFound; }

        NO_DISCARD
        bool IsCorruption() const { return code() == kCorruption; }

        NO_DISCARD
        bool IsNotSupported() const { return code() == kNotSupported; }

        NO_DISCARD
        bool IsInvalidArgument() const { return code() == kInvalidArgument; }

        NO_DISCARD
        bool IsIOError() const { return code() == kIOError; }

        NO_DISCARD
        std::string ToString() const;

        //static:
        static Status OK() { return Status(); }

        static Status NotFound(const Slice &msg, const Slice &msg2 = Slice()) {
            return Status(kNotFound, msg, msg2);
        }

        static Status Corruption(const Slice &msg, const Slice &msg2 = Slice()) {
            return Status(kCorruption, msg, msg2);
        }

        static Status NotSupported(const Slice &msg, const Slice &msg2 = Slice()) {
            return Status(kNotSupported, msg, msg2);
        }

        static Status InvalidArgument(const Slice &msg, const Slice &msg2 = Slice()) {
            return Status(kInvalidArgument, msg, msg2);
        }

        static Status IOError(const Slice &msg, const Slice &msg2 = Slice()) {
            return Status(kIOError, msg, msg2);
        }

    private:
        enum Code {
            kOk = 0,
            kNotFound = 1,
            kCorruption = 2,
            kNotSupported = 3,
            kInvalidArgument = 4,
            kIOError = 5
        };

        NO_DISCARD
        Code code() const {
            return state_ == nullptr ? kOk : static_cast<Code>(state_[4]);
        }

        Status(Code code, const Slice &msg, const Slice &msg2);

        static const char *CopyState(const char *s);


        // state_[0,3]   == length of message
        // state_[4]     == code
        // state_[5..]   == message
        const char *state_;
    };

}

#endif //MY_LEVELDB_STATUS_H
