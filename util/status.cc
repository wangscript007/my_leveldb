//
// Created by kuiper on 2021/2/9.
//

#include "leveldb/status.h"
#include <algorithm>

namespace leveldb {

    Status::Status(Status::Code code, const Slice &msg, const Slice &msg2) {

    }

    // private static
    const char *Status::CopyState(const char *s) {
        uint32_t size;
        std::memcpy(&size, s, sizeof(size));
        char *result = new char[size + 5];
        std::memcpy(result, s, size + 5);
        return result;
    }

    Status::Status(const Status &rhs) {
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }

    Status &Status::operator=(const Status &rhs) {
        // Clang tidy
        if (this == &rhs && state_ != rhs.state_) {
            delete[] state_;
            state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
        }
        return *this;
    }

    Status::Status(Status &&rhs) noexcept {
        state_ = rhs.state_;
        rhs.state_ = nullptr;
    }

    Status &Status::operator=(Status &&rhs) noexcept {
        std::swap(state_, rhs.state_);
        return *this;
    }

    std::string Status::ToString() const {
        if (state_ == nullptr) {
            return "OK";
        }
        char tmp[30];
        const char *type;
        switch (code()) {
            case kOk:
                type = "OK";
                break;
            case kNotFound:
                type = "NotFound: ";
                break;
            case kCorruption:
                type = "Corruption: ";
                break;
            case kNotSupported:
                type = "Not Supported: ";
                break;
            case kInvalidArgument:
                type = "Invalid Argument: ";
                break;
            case kIOError:
                type = "IO error: ";
                break;
            default:
                std::snprintf(tmp, sizeof(tm), "Unknown code(%d): ", static_cast<int>(code()));
                type = tmp;
                break;
        }
        std::string res(type);
        uint32_t len;
        std::memcpy(&len, state_, sizeof(len));
        res.append(state_ + 5, len);
        return res;
    }




}
