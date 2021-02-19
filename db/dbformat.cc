//
// Created by kuiper on 2021/2/18.
//

#include "db/dbformat.h"

#include <sstream>

namespace leveldb {

    std::string ParsedInternalKey::DebugString() const {
        std::ostringstream ss;
        ss << '\'' << EscapeString(user_key.ToString()) << "' @ " << sequence << " : "
           << static_cast<int>(type);
        return ss.str();
    }

    std::string InternalKey::DebugString() const {
        ParsedInternalKey parsed;
        if (ParseInternalKey(rep_, &parsed)) {
            return parsed.DebugString();
        }

        std::ostringstream ss;
        ss << "(bad)" << EscapeString(rep_);
        return ss.str();
    }

    int InternalKeyComparator::Compare(const Slice &a, const Slice &b) const {
        // fixme
        return 1;
    }

    void InternalKeyComparator::FindShortestSeparator(std::string *start, const Slice &limit) const {

    }

    void InternalKeyComparator::FindShortSuccessor(std::string *key) const {

    }

    // fixme FilterPolicy


    LookupKey::LookupKey(const Slice &user_key, SequenceNumber sequence) {
        size_t usize = user_key.size();
        size_t needed = usize + 13;

        char *dst;
        if (needed < sizeof(space_)) {
            dst = space_;
        } else {
            dst = new char[needed];
        }

        start_ = dst;

        // 先写入key的varint长度: user_key + 8byte(seq+type)
        dst = EncodeVarint32(dst, usize + 8);
        kstart_ = dst;
        // 写入user_key
        std::memcpy(dst, user_key.data(), usize);
        dst += usize;
        // 写入8byte的seq和type
        EncodeFixed64(dst, PackSequenceAndType(sequence, kValueTypeForSeek));
        dst += 8;
        end_ = dst;
    }

}
