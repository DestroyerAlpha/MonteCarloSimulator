#pragma once
#include <cmath>
#include <vector>

namespace montecarlo {
struct Result {
    double estimate{};
    double variance{};
    double standard_error{};
    std::uint64_t iterations{};
    double elapsed_ms{};
};

struct ConfidenceInterval {
    double lower;
    double upper;
    double confidence_level;
};

inline ConfidenceInterval ci_95(const Result& r) {
    constexpr double z = 1.96;
    return {
        r.estimate - z * r.standard_error,
        r.estimate + z * r.standard_error,
        0.95
    };
}

inline ConfidenceInterval confidence_interval(const Result& r, double level) {
    // Simplified: use normal approximation z-scores
    // For production, use proper t-distribution
    double z = 1.96; // 95%
    if (level >= 0.99) z = 2.576;
    else if (level >= 0.95) z = 1.96;
    else if (level >= 0.90) z = 1.645;

    return {
        r.estimate - z * r.standard_error,
        r.estimate + z * r.standard_error,
        level
    };
}

// Welford's online variance aggregator
template<typename T = double>
class WelfordAggregator {
 public:
    void add(T value) {
        count_++;
        double delta = value - mean_;
        mean_ += delta / static_cast<double>(count_);
        double delta2 = value - mean_;
        m2_ += delta * delta2;
    }

    double result() const {
        return mean_;
    }

    double variance() const {
        return count_ > 1 ? m2_ / static_cast<double>(count_ - 1) : 0.0;
    }

    double std_error() const {
        return count_ > 0 ? std::sqrt(variance() / static_cast<double>(count_)) : 0.0;
    }

    // Merge another aggregator using Chan's parallel variance algorithm
    void merge(const WelfordAggregator& other) {
        if (other.count_ == 0) return;
        if (count_ == 0) {
            mean_ = other.mean_;
            m2_ = other.m2_;
            count_ = other.count_;
            return;
        }
        double total = static_cast<double>(count_ + other.count_);
        double delta = other.mean_ - mean_;
        mean_ += delta * (static_cast<double>(other.count_) / total);
        m2_ += other.m2_ + delta * delta *
            (static_cast<double>(count_) * static_cast<double>(other.count_) / total);
        count_ += other.count_;
    }

    void reset() {
        mean_ = 0.0;
        m2_ = 0.0;
        count_ = 0;
    }

    std::uint64_t count() const { return count_; }

 private:
    double mean_ = 0.0;
    double m2_ = 0.0;
    std::uint64_t count_ = 0;
};

template<typename T = double>
class HistogramAggregator {
 public:
    explicit HistogramAggregator(size_t bins = 100, double min = 0.0,
        double max = 1.0): bins_(bins, 0), min_(min), max_(max),
        bin_width_((max - min) / bins) {}

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
