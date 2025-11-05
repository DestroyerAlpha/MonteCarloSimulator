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

TEST(testUniformRealDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    // Generate many samples
    double sum = 0.0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        double val = rng.uniform(0.0, 10.0);
        ASSERT_TRUE(val >= 0.0 && val <= 10.0);
        sum += val;
    }
    
    double mean = sum / samples;
    ASSERT_NEAR(mean, 5.0, 0.2);  // Mean of uniform [0, 10]
    
    std::cout << "✓ testUniformRealDistribution passed\n";
}

TEST(testUniformIntDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    // Generate many samples
    int sum = 0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        int val = rng.uniform(1, 6);
        ASSERT_TRUE(val >= 1 && val <= 6);
        sum += val;
    }
    
    double mean = static_cast<double>(sum) / samples;
    ASSERT_NEAR(mean, 3.5, 0.1);  // Mean of uniform [1, 6]
    
    std::cout << "✓ testUniformIntDistribution passed\n";
}

TEST(testNormalDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    double sum = 0.0;
    double sum_sq = 0.0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        double val = rng.normal(5.0, 2.0);
        sum += val;
        sum_sq += val * val;
    }
    
    double mean = sum / samples;
    double variance = (sum_sq / samples) - (mean * mean);
    double stddev = std::sqrt(variance);
    
    ASSERT_NEAR(mean, 5.0, 0.1);
    ASSERT_NEAR(stddev, 2.0, 0.1);
    
    std::cout << "✓ testNormalDistribution passed\n";
}

TEST(testExponentialDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    double lambda = 2.0;
    double sum = 0.0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        double val = rng.exponential(lambda);
        ASSERT_TRUE(val >= 0.0);
        sum += val;
    }
    
    double mean = sum / samples;
    // Mean of exponential distribution is 1/lambda
    ASSERT_NEAR(mean, 1.0 / lambda, 0.05);
    
    std::cout << "✓ testExponentialDistribution passed\n";
}

TEST(testPoissonDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    double expected_mean = 5.0;
    int sum = 0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        int val = rng.poisson(expected_mean);
        ASSERT_TRUE(val >= 0);
        sum += val;
    }
    
    double mean = static_cast<double>(sum) / samples;
    ASSERT_NEAR(mean, expected_mean, 0.2);
    
    std::cout << "✓ testPoissonDistribution passed\n";
}

TEST(testBinomialDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    int n = 10;
    double p = 0.5;
    int sum = 0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        int val = rng.binomial(n, p);
        ASSERT_TRUE(val >= 0 && val <= n);
        sum += val;
    }
    
    double mean = static_cast<double>(sum) / samples;
    // Mean of binomial is n*p
    ASSERT_NEAR(mean, n * p, 0.2);
    
    std::cout << "✓ testBinomialDistribution passed\n";
}

TEST(testGammaDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    double alpha = 2.0;
    double beta = 2.0;
    double sum = 0.0;
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        double val = rng.gamma(alpha, beta);
        ASSERT_TRUE(val >= 0.0);
        sum += val;
    }
    
    double mean = sum / samples;
    // Mean of gamma distribution is alpha * beta
    ASSERT_NEAR(mean, alpha * beta, 0.2);
    
    std::cout << "✓ testGammaDistribution passed\n";
}

TEST(testLogNormalDistribution) {
    montecarlo::RandomGenerator rng(12345);
    
    const int samples = 10000;
    
    for (int i = 0; i < samples; ++i) {
        double val = rng.logNormal(0.0, 1.0);
        ASSERT_TRUE(val > 0.0);  // Log-normal is always positive
    }
    
    std::cout << "✓ testLogNormalDistribution passed\n";
}

TEST(testThreadLocalGenerator) {
    // Test that thread-local generator works
    auto& rng = montecarlo::getThreadLocalGenerator();
    
    double val = rng.uniform(0.0, 1.0);
    ASSERT_TRUE(val >= 0.0 && val <= 1.0);
    
    std::cout << "✓ testThreadLocalGenerator passed\n";
}

TEST(testBoxMuller) {
    montecarlo::RandomGenerator rng(12345);
    
    double sum1 = 0.0, sum2 = 0.0;
    const int samples = 5000;  // We get 2 values per call
    
    for (int i = 0; i < samples; ++i) {
        auto [z1, z2] = montecarlo::boxMuller(rng);
        sum1 += z1;
        sum2 += z2;
    }
    
    double mean1 = sum1 / samples;
    double mean2 = sum2 / samples;
    
    // Both should be approximately N(0,1)
    ASSERT_NEAR(mean1, 0.0, 0.1);
    ASSERT_NEAR(mean2, 0.0, 0.1);
    
    std::cout << "✓ testBoxMuller passed\n";
}

TEST(testSeeding) {
    montecarlo::RandomGenerator rng1(12345);
    montecarlo::RandomGenerator rng2(12345);
    
    // Same seed should produce same sequence
    for (int i = 0; i < 10; ++i) {
        ASSERT_NEAR(rng1.uniform(0.0, 1.0), rng2.uniform(0.0, 1.0), 0.0001);
    }
    
    // Re-seed and verify
    rng1.seed(54321);
    rng2.seed(54321);
    
    for (int i = 0; i < 10; ++i) {
        ASSERT_NEAR(rng1.normal(0.0, 1.0), rng2.normal(0.0, 1.0), 0.0001);
    }
    
    std::cout << "✓ testSeeding passed\n";
}

int main() {
    std::cout << "Running Random Generator Tests\n";
    std::cout << "===============================\n\n";
    
    testUniformRealDistribution();
    testUniformIntDistribution();
    testNormalDistribution();
    testExponentialDistribution();
    testPoissonDistribution();
    testBinomialDistribution();
    testGammaDistribution();
    testLogNormalDistribution();
    testThreadLocalGenerator();
    testBoxMuller();
    testSeeding();
    
    std::cout << "\n✅ All tests passed!\n";
    return 0;
}
