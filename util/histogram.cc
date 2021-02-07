//
// Created by kuiper on 2021/2/7.
//

#include "histogram.h"

namespace leveldb {

    Histogram::Histogram() {

    }

    Histogram::~Histogram() {

    }

    void Histogram::Add(double value) {

    }

    void Histogram::Clear() {

    }

    void Histogram::Merge(const Histogram &other) {

    }

    std::string Histogram::ToString() const {
        return "";
    }

    double Histogram::Median() const {
        return 0.0;
    }

    double Histogram::Average() const {
        return 0;
    }

    double Histogram::Percentile(double p) const {
        return 0;
    }

    double Histogram::StandardDeviation() const {
        return 0;
    }
}