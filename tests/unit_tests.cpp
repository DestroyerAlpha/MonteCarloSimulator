#include "montecarlo/montecarlo.hpp"
#include "stub_rng.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>

using namespace montecarlo;

struct TestFailure : public std::runtime_error {
    explicit TestFailure(const std::string& msg) : std::runtime_error(msg) {}
};

// Minimal assertion helpers to keep dependencies light
#define EXPECT_TRUE(cond, msg)                                                     \
    do {                                                                          \
        if (!(cond)) {                                                            \
            std::ostringstream oss;                                               \
            oss << "EXPECT_TRUE failed: " << msg;                                 \
            throw TestFailure(oss.str());                                         \
        }                                                                         \
    } while (0)

#define EXPECT_NEAR(val, exp, tol, msg)                                           \
    do {                                                                          \
        auto v_ = (val);                                                          \
        auto e_ = (exp);                                                          \
        auto t_ = (tol);                                                          \
        if (std::abs(v_ - e_) > t_) {                                             \
            std::ostringstream oss;                                               \
            oss << "EXPECT_NEAR failed: got " << v_ << " expected " << e_         \
                << " tol " << t_ << " (" << msg << ")";                           \
            throw TestFailure(oss.str());                                         \
        }                                                                         \
    } while (0)

#define EXPECT_EQ(val, exp, msg)                                                  \
    do {                                                                          \
        auto v_ = (val);                                                          \
        auto e_ = (exp);                                                          \
        if (!(v_ == e_)) {                                                        \
            std::ostringstream oss;                                               \
            oss << "EXPECT_EQ failed: got " << v_ << " expected " << e_           \
                << " (" << msg << ")";                                            \
            throw TestFailure(oss.str());                                         \
        }                                                                         \
    } while (0)

// Test Models
struct ConstantOneModel {
    template <typename RNG>
    double operator()(RNG& rng) const {
        (void)rng;
        return 1.0;
    }
};

// RNG that increments on each call for deterministic sequences
struct IncrementingRng {
    using result_type = std::uint64_t;
    explicit IncrementingRng(std::uint64_t start = 0) : state(start) {}
    result_type operator()() { return state++; }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
    std::uint64_t state;
};

// Factory that seeds the incrementing RNG
struct IncrementingFactory {
    IncrementingRng operator()(std::uint64_t seed) const noexcept {
        return IncrementingRng(seed);
    }
};

// Simple uniform [0,1) sampler
struct Uniform01Model {
    template <typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    }
};

// Test cases
// Verify Welford stats on a small fixed dataset
void test_welford_basic_stats() {
    WelfordAggregator<> agg;
    std::vector<double> values{1.0, 2.0, 3.0, 4.0};
    for (double v : values) {
        agg.add(v);
    }

    EXPECT_EQ(agg.count(), values.size(), "count tracks inserted samples");
    EXPECT_NEAR(agg.result(), 2.5, 1e-12, "mean of 1..4");
    EXPECT_NEAR(agg.variance(), 1.6666666667, 1e-9, "sample variance of 1..4");
    double expected_stderr = std::sqrt(agg.variance() / static_cast<double>(values.size()));
    EXPECT_NEAR(agg.std_error(), expected_stderr, 1e-12, "std error derived from variance");
}

// Ensure reset clears all running state
void test_welford_reset() {
    WelfordAggregator<> agg;
    agg.add(1.0);
    agg.add(2.0);
    agg.reset();

    EXPECT_EQ(agg.count(), 0u, "reset clears count");
    EXPECT_NEAR(agg.result(), 0.0, 1e-12, "reset clears mean");
    EXPECT_NEAR(agg.variance(), 0.0, 1e-12, "reset clears variance");
}

// Same seed should produce identical sequences
void test_rng_reproducibility() {
    constexpr std::uint64_t seed = 42;
    auto rng1 = make_rng(seed);
    auto rng2 = make_rng(seed);

    std::vector<std::uint64_t> seq1(5);
    std::vector<std::uint64_t> seq2(5);
    for (int i = 0; i < 5; ++i) {
        seq1[i] = rng1();
        seq2[i] = rng2();
    }

    EXPECT_TRUE(seq1 == seq2, "same seed produces identical stream");
}

// Different stream ids should not match
void test_rng_stream_independence() {
    constexpr std::uint64_t seed = 42;
    auto rng1 = make_rng(seed, 1);
    auto rng2 = make_rng(seed, 2);

    bool all_equal = true;
    for (int i = 0; i < 5; ++i) {
        if (rng1() != rng2()) {
            all_equal = false;
            break;
        }
    }

    EXPECT_TRUE(!all_equal, "different stream ids should decorrelate sequences");
}

// Basic mean/variance sanity check for uniform distribution
void test_rng_uniform_sanity() {
    std::uint64_t seed = 2024;
    auto rng = make_rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    constexpr std::size_t samples = 50'000;
    double mean = 0.0;
    double m2 = 0.0;
    for (std::size_t i = 0; i < samples; ++i) {
        double x = dist(rng);
        double delta = x - mean;
        mean += delta / static_cast<double>(i + 1);
        double delta2 = x - mean;
        m2 += delta * delta2;
    }
    double variance = m2 / static_cast<double>(samples - 1);

    EXPECT_NEAR(mean, 0.5, 0.01, "uniform(0,1) sample mean");
    EXPECT_NEAR(variance, 1.0 / 12.0, 0.01, "uniform(0,1) variance");
}

// Constant model should produce zero variance and mean 1.0
void test_sequential_constant_model() {
    ConstantOneModel model;
    auto engine = make_engine(model, execution::Sequential{}, 1234ULL, StubFactory{}, transform::Identity{});
    auto result = engine.run(5'000);

    EXPECT_NEAR(result.estimate, 1.0, 1e-12, "constant model mean");
    EXPECT_NEAR(result.variance, 0.0, 1e-12, "constant model variance");
    EXPECT_EQ(result.iterations, 5'000u, "iterations tracked");
}

// Deterministic RNG sequence should yield a predictable mean/variance
void test_sequential_deterministic_sequence() {
    // Sequence should be 10,11,12,13,... starting at seed
    IncrementingFactory factory;
    auto model = [](IncrementingRng& rng) {
        return static_cast<double>(rng()) / 10.0;
    };

    constexpr std::uint64_t seed = 10;
    constexpr std::uint64_t n = 4;
    auto engine = make_engine(model, execution::Sequential{}, seed, factory, transform::Identity{});
    auto result = engine.run(n);

    // Values: 10/10, 11/10, 12/10, 13/10 => mean = 1.15
    EXPECT_NEAR(result.estimate, 1.15, 1e-12, "deterministic stream mean");
    // Sample variance of {1.0,1.1,1.2,1.3}
    EXPECT_NEAR(result.variance, 0.0166666667, 1e-9, "deterministic stream variance");
    EXPECT_EQ(result.iterations, n, "iteration count");
}

// Running parallel twice with the same seed should be deterministic
void test_parallel_reproducibility() {
#ifdef MCLIB_PARALLEL_ENABLED
    Uniform01Model model;
    constexpr std::uint64_t seed = 555;
    constexpr std::uint64_t n = 50'000;

    auto engine = make_parallel_engine(model, std::thread::hardware_concurrency(), seed);
    auto r1 = engine.run(n);
    auto r2 = engine.run(n);

    EXPECT_NEAR(r1.estimate, r2.estimate, 1e-12, "parallel runs deterministic per seed");
    EXPECT_NEAR(r1.variance, 1.0 / 12.0, 0.01, "variance preserved through parallel merge");
#else
    std::cout << "[skip] parallel reproducibility (MCLIB_PARALLEL_ENABLED=OFF)\n";
#endif
}

// Changing the seed should change the result even in parallel
void test_parallel_seed_variation_changes_result() {
#ifdef MCLIB_PARALLEL_ENABLED
    Uniform01Model model;
    constexpr std::uint64_t n = 20'000;

    auto e1 = make_parallel_engine(model, 2, 42);
    auto e2 = make_parallel_engine(model, 2, 43);

    auto r1 = e1.run(n);
    auto r2 = e2.run(n);

    EXPECT_TRUE(std::abs(r1.estimate - r2.estimate) > 1e-4, "changing seed changes estimate");
#else
    std::cout << "[skip] parallel seed variation (MCLIB_PARALLEL_ENABLED=OFF)\n";
#endif
}

// runner plumbing
struct TestCase {
    const char* name;
    std::function<void()> fn;
};

int main() {
    std::vector<TestCase> tests = {
        {"welford_basic_stats", test_welford_basic_stats},
        {"welford_reset", test_welford_reset},
        {"rng_reproducibility", test_rng_reproducibility},
        {"rng_stream_independence", test_rng_stream_independence},
        {"rng_uniform_sanity", test_rng_uniform_sanity},
        {"sequential_constant_model", test_sequential_constant_model},
        {"sequential_deterministic_sequence", test_sequential_deterministic_sequence},
        {"parallel_reproducibility", test_parallel_reproducibility},
        {"parallel_seed_variation", test_parallel_seed_variation_changes_result},
    };

    std::size_t failures = 0;
    for (const auto& t : tests) {
        try {
            t.fn();
            std::cout << "[PASS] " << t.name << "\n";
        } catch (const TestFailure& e) {
            ++failures;
            std::cerr << "[FAIL] " << t.name << ": " << e.what() << "\n";
        } catch (const std::exception& e) {
            ++failures;
            std::cerr << "[FAIL] " << t.name << " threw exception: " << e.what() << "\n";
        }
    }

    std::cout << "Ran " << tests.size() << " tests; failures=" << failures << "\n";
    return failures == 0 ? 0 : 1;
}
