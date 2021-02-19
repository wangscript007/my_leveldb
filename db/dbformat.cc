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
        int r = user_comparator_->Compare(ExtractUserKey(a), ExtractUserKey(b));
        // 如果key相同 则比较seq号  seq号低8位都是type
        // 排序规则:
        // user_key按照用户提供的升序
        // seqNbr: 降序  NOTE !!!!
        // type  : 降序  NOTE !!!!
        // 不允许user_key+seqNbr+type完全相同.
        // 如果user_key和seqNbr相同, type不同, 则kTypeValue最高优先级
        // 即先add_kTypeValue，再add_kTypeDeletion 则删除无效, 但是逻辑上不可能出现这种场景.
        if (r == 0) {
            const int64_t anum = DecodeFixed64(a.data() + a.size() - 8);
            const int64_t bnum = DecodeFixed64(b.data() + b.size() - 8);
            if (anum > bnum) {
                r = -1;
            } else if (anum < bnum) {
                r = +1;
            }
        }
        return r;
    }

    void InternalKeyComparator::FindShortestSeparator(std::string *start, const Slice &limit) const {

    }

    void InternalKeyComparator::FindShortSuccessor(std::string *key) const {

    }

    // fixme FilterPolicy


    LookupKey::LookupKey(const Slice &user_key, SequenceNumber sequence) {
        size_t usize = user_key.size();
        size_t needed = usize + 13; // 13 = 最大的varint占5个字节 + 8个字节的seq<<56 | type

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
