//
// Created by kuiper on 2021/2/12.
//

#ifndef MY_LEVELDB_WRITE_BATCH_H
#define MY_LEVELDB_WRITE_BATCH_H

#include "leveldb/export.h"
#include "leveldb/status.h"

namespace leveldb {

    class Slice;

    class LEVELDB_EXPORT WriteBatch {
    public:
        class LEVELDB_EXPORT Handler {
        public:
            virtual ~Handler();

            virtual void Put(const Slice &key, const Slice &val) = 0;

            virtual void Delete(const Slice &key) = 0;
        };

        WriteBatch();

        WriteBatch(const WriteBatch &) = default;
        WriteBatch &operator=(const WriteBatch &) = default;

        ~WriteBatch();

        void Put(const Slice &key, const Slice &value);

        void Delete(const Slice &key);

        void Clear();

        // @brief 近似的大小
        size_t ApproximateSize() const;

        void Append(const WriteBatch &source);

        Status Iterate(Handler *handler) const;

    private:
        friend class WriteBatchInternal;

        std::string rep_;
    };

}

#endif //MY_LEVELDB_WRITE_BATCH_H
