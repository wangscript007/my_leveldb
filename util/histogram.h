//
// Created by kuiper on 2021/2/7.
//

#ifndef MY_LEVELDB_HISTOGRAM_H
#define MY_LEVELDB_HISTOGRAM_H

#include <string>
#include "leveldb/cxx.h"

namespace leveldb {

    // 直方图
    class Histogram {
    public:
        Histogram();

        ~Histogram();

        void Clear();

        void Add(double value);

        void Merge(const Histogram &other);

        NO_DISCARD std::string ToString() const;

    private:
        enum {
            kNumBuckets = 154
        };

        NO_DISCARD double Median() const;
        NO_DISCARD double Percentile(double p) const;
        NO_DISCARD double Average() const;
        NO_DISCARD double StandardDeviation() const;

        static const double kBucketLimit[kNumBuckets];

        double min_;
        double max_;
        double num_;
        double sum_;
        double sum_squares_;

        double buckets_[kNumBuckets];
    };

}


#endif //MY_LEVELDB_HISTOGRAM_H
