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

    }

    void InternalKeyComparator::FindShortestSeparator(std::string *start, const Slice &limit) const {

    }

    void InternalKeyComparator::FindShortSuccessor(std::string *key) const {

    }

    // fixme FilterPolicy

}
