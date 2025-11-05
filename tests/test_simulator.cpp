#include <montecarlo/montecarlo.h>
#include <iostream>
#include <cmath>
#include <cassert>

// Simple test framework
#define TEST(name) void name()
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                  << " - Assertion failed: " << #condition << "\n"; \
        return; \
    }
#define ASSERT_NEAR(a, b, tolerance) \
    if (std::abs((a) - (b)) > (tolerance)) { \
        std::cerr << "FAIL: " << __FILE__ << ":" << __LINE__ \
                  << " - Values not close: " << (a) << " vs " << (b) \
                  << " (tolerance: " << (tolerance) << ")\n"; \
        return; \
    }

TEST(testBasicSimulation) {
    montecarlo::MonteCarloSimulator<double> sim(1000);
    
    // Simple simulation that always returns 5.0
    auto result = sim.run([]() { return 5.0; });
    
    ASSERT_NEAR(result.mean, 5.0, 0.0001);
    ASSERT_NEAR(result.std_dev, 0.0, 0.0001);
    ASSERT_NEAR(result.min, 5.0, 0.0001);
    ASSERT_NEAR(result.max, 5.0, 0.0001);
    ASSERT_TRUE(result.num_samples == 1000);
    
    std::cout << "✓ testBasicSimulation passed\n";
}

TEST(testUniformDistribution) {
    montecarlo::MonteCarloSimulator<double> sim(10000);
    
    // Generate uniform random numbers between 0 and 10
    auto result = sim.run([]() {
        auto& rng = montecarlo::getThreadLocalGenerator();
        return rng.uniform(0.0, 10.0);
    });
    
    // Mean should be around 5.0 for uniform [0, 10]
    ASSERT_NEAR(result.mean, 5.0, 0.2);
    
    // Std dev for uniform [a, b] is (b-a)/sqrt(12) ≈ 2.887
    ASSERT_NEAR(result.std_dev, 2.887, 0.2);
    
    // Min and max should be close to bounds
    ASSERT_TRUE(result.min >= 0.0 && result.min < 1.0);
    ASSERT_TRUE(result.max > 9.0 && result.max <= 10.0);
    
    std::cout << "✓ testUniformDistribution passed\n";
}

TEST(testNormalDistribution) {
    montecarlo::MonteCarloSimulator<double> sim(10000);
    
    // Generate normal random numbers with mean=10, stddev=2
    auto result = sim.run([]() {
        auto& rng = montecarlo::getThreadLocalGenerator();
        return rng.normal(10.0, 2.0);
    });
    
    // Mean should be around 10
    ASSERT_NEAR(result.mean, 10.0, 0.1);
    
    // Std dev should be around 2
    ASSERT_NEAR(result.std_dev, 2.0, 0.1);
    
    std::cout << "✓ testNormalDistribution passed\n";
}

TEST(testPiEstimation) {
    montecarlo::MonteCarloSimulator<double> sim(100000);
    
    auto pi_simulation = []() -> double {
        auto& rng = montecarlo::getThreadLocalGenerator();
        double x = rng.uniform(0.0, 1.0);
        double y = rng.uniform(0.0, 1.0);
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    };
    
    auto result = sim.run(pi_simulation);
    double pi_estimate = 4.0 * result.mean;
    
    // Should be within 0.05 of actual pi
    ASSERT_NEAR(pi_estimate, M_PI, 0.05);
    
    std::cout << "✓ testPiEstimation passed (estimated: " << pi_estimate << ")\n";
}

TEST(testSequentialVsParallel) {
    const size_t num_sims = 10000;
    montecarlo::MonteCarloSimulator<double> sim(num_sims);
    
    auto test_func = []() {
        auto& rng = montecarlo::getThreadLocalGenerator();
        return rng.uniform(0.0, 1.0);
    };
    
    // Run sequential
    auto result_seq = sim.run(test_func, false);
    
    // Run parallel
    auto result_par = sim.run(test_func, true);
    
    // Both should give similar results (within reasonable tolerance)
    ASSERT_NEAR(result_seq.mean, result_par.mean, 0.1);
    ASSERT_TRUE(result_seq.num_samples == result_par.num_samples);
    
    std::cout << "✓ testSequentialVsParallel passed\n";
}

TEST(testConfidenceInterval) {
    montecarlo::MonteCarloSimulator<double> sim(10000);
    
    // Generate normal distribution
    auto result = sim.run([]() {
        auto& rng = montecarlo::getThreadLocalGenerator();
        return rng.normal(0.0, 1.0);
    });
    
    // Confidence interval should contain the mean
    ASSERT_TRUE(result.confidence_interval_lower <= result.mean);
    ASSERT_TRUE(result.confidence_interval_upper >= result.mean);
    
    // For normal(0,1), the true mean is 0, which should be in CI
    ASSERT_TRUE(result.confidence_interval_lower <= 0.0);
    ASSERT_TRUE(result.confidence_interval_upper >= 0.0);
    
    std::cout << "✓ testConfidenceInterval passed\n";
}

TEST(testIntegerSimulation) {
    montecarlo::MonteCarloSimulator<int> sim(50000);
    
    // Simulate dice rolls
    auto result = sim.run([]() {
        auto& rng = montecarlo::getThreadLocalGenerator();
        return rng.uniform(1, 6);
    });
    
    // Mean should be around 3.5 for fair dice (but will be integer)
    // With large sample, it should be very close to 3 or 4
    ASSERT_TRUE(result.mean >= 3 && result.mean <= 4);
    
    // Min and max
    ASSERT_TRUE(result.min >= 1 && result.min <= 6);
    ASSERT_TRUE(result.max >= 1 && result.max <= 6);
    
    std::cout << "✓ testIntegerSimulation passed (mean: " << result.mean << ")\n";
}

TEST(testRawResults) {
    montecarlo::MonteCarloSimulator<double> sim(100);
    
    auto results = sim.runRaw([]() { return 42.0; });
    
    ASSERT_TRUE(results.size() == 100);
    
    for (double val : results) {
        ASSERT_NEAR(val, 42.0, 0.0001);
    }
    
    std::cout << "✓ testRawResults passed\n";
}

int main() {
    std::cout << "Running Monte Carlo Simulator Tests\n";
    std::cout << "====================================\n\n";
    
    testBasicSimulation();
    testUniformDistribution();
    testNormalDistribution();
    testPiEstimation();
    testSequentialVsParallel();
    testConfidenceInterval();
    testIntegerSimulation();
    testRawResults();
    
    std::cout << "\n✅ All tests passed!\n";
    return 0;
}
