#pragma once
#include <cmath>
#include <algorithm>

namespace montecarlo::transform {

// Identity transform (no transformation)
struct Identity {
    double operator()(double x) const noexcept {
        return x;
    }
};

// Square transform
struct Square {
    double operator()(double x) const noexcept {
        return x * x;
    }
};

// Absolute value transform
struct Abs {
    double operator()(double x) const noexcept {
        return std::abs(x);
    }
};

// Natural logarithm (with offset to handle negative values)
struct Log {
    explicit Log(double offset = 1.0) : offset_(offset) {}

    double operator()(double x) const noexcept {
        return std::log(x + offset_);
    }

 private:
    double offset_;
};

// Exponential transform
struct Exp {
    double operator()(double x) const noexcept {
        return std::exp(x);
    }
};

// Indicator function (returns 1 if condition met, 0 otherwise)
struct Indicator {
    explicit Indicator(double threshold, bool greater_than = true)
        : threshold_(threshold), greater_than_(greater_than) {}

    double operator()(double x) const noexcept {
        return (greater_than_ ? (x > threshold_) : (x < threshold_)) ? 1.0 : 0.0;
    }

 private:
    double threshold_;
    bool greater_than_;
};

// Clamp to range [min, max]
struct Clamp {
    Clamp(double min, double max) : min_(min), max_(max) {}

    double operator()(double x) const noexcept {
        return std::clamp(x, min_, max_);
    }

 private:
    double min_;
    double max_;
};

// Linear scaling: a * x + b
struct LinearScale {
    LinearScale(double a = 1.0, double b = 0.0) : a_(a), b_(b) {}

    double operator()(double x) const noexcept {
        return a_ * x + b_;
    }

 private:
    double a_;
    double b_;
};

// Power transform
struct Power {
    explicit Power(double exponent) : exponent_(exponent) {}

    double operator()(double x) const noexcept {
        return std::pow(x, exponent_);
    }

 private:
    double exponent_;
};

// Sigmoid (logistic function)
struct Sigmoid {
    double operator()(double x) const noexcept {
        return 1.0 / (1.0 + std::exp(-x));
    }
};

// Compose two transforms: f(g(x))
template<typename F, typename G>
struct Compose {
    Compose(F f, G g) : f_(f), g_(g) {}

    double operator()(double x) const {
        return f_(g_(x));
    }

 private:
    F f_;
    G g_;
};

// Helper function to compose transforms
template<typename F, typename G>
auto compose(F f, G g) {
    return Compose<F, G>(f, g);
}

} // namespace montecarlo::transform

