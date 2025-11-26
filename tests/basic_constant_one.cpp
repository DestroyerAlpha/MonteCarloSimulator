#include <gtest/gtest.h>
#include <cmath>
#include <iostream>
#include "montecarlo/montecarlo.hpp"

using namespace montecarlo;

// Simple model that always returns 1
struct ConstantOneModel {
    template <typename Rng>
    double trial(Rng& rng) const {
        (void)rng;
        return 1.0;
    }
};

// Test fixture (optional here, useful if you want shared setup)
class MonteCarloTest : public ::testing::Test {
 protected:
    void SetUp() override {
        model = ConstantOneModel{};
        base_seed = 424242ULL;
        N = 1'000'000;
    }

    ConstantOneModel model;
    std::uint64_t base_seed;
    std::uint64_t N;
};

// Test the Monte Carlo mean
TEST_F(MonteCarloTest, ConstantOneMean) {
    // Default transform (identity)
    auto transform = [](double x) { return x; };

    // Construct engine
    auto engine = make_sequential_engine(model, base_seed,  transform);

    // Run simulation
    auto result = engine.run(N);

    // Print debug info (optional)
    std::cout << "mean=" << result.estimate
              << " var=" << result.variance
              << " stderr=" << result.standard_error << std::endl;

    // Check mean is close to expected
    EXPECT_NEAR(result.estimate, 1.0, 1e-3);
}

// Optionally test variance/stderr
TEST_F(MonteCarloTest, ConstantOneVariance) {
    // Default transform (identity)
    auto transform = [](double x) { return x; };

    // Construct engine
    auto engine = make_sequential_engine(model, base_seed,  transform);
    
    // Run simulation
    auto result = engine.run(N);

    // Variance of constant 1 should be zero
    EXPECT_NEAR(result.variance, 0.0, 1e-12);
    EXPECT_NEAR(result.standard_error, 0.0, 1e-12);
}
