#pragma once
#include <cmath>
#include <vector>

namespace montecarlo {

template<typename T = double>
class MeanAggregator {
 public:
    void add(T value) {
        sum_ += value;
        sum_sq_ += value * value;
        count_++;
    }

    double result() const {
        return count_ > 0 ? sum_ / count_ : 0.0;
    }

    double variance() const {
        if (count_ < 2) return 0.0;
        double mean = result();
        return (sum_sq_ / count_ - mean * mean) * count_ / (count_ - 1);
    }

    double std_error() const {
        return std::sqrt(variance() / count_);
    }

    void reset() {
        sum_ = 0;
        sum_sq_ = 0;
        count_ = 0;
    }

    size_t count() const { return count_; }

 private:
    double sum_ = 0.0;
    double sum_sq_ = 0.0;
    size_t count_ = 0;
};

template<typename T = double>
class HistogramAggregator {
 public:
    explicit HistogramAggregator(size_t bins = 100, double min = 0.0, double max = 1.0)
        : bins_(bins, 0), min_(min), max_(max), bin_width_((max - min) / bins) {}
    
    void add(T value) {
        if (value >= min_ && value < max_) {
            size_t idx = static_cast<size_t>((value - min_) / bin_width_);
            if (idx < bins_.size()) {
                bins_[idx]++;
            }
        }
        count_++;
    }

    double result() const {
        return count_ > 0 ? static_cast<double>(count_) : 0.0;
    }

    const std::vector<size_t>& histogram() const { return bins_; }

    void reset() {
        std::fill(bins_.begin(), bins_.end(), 0);
        count_ = 0;
    }

 private:
    std::vector<size_t> bins_;
    double min_, max_, bin_width_;
    size_t count_ = 0;
};

} // namespace montecarlo
