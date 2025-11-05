#pragma once

#include <random>
#include <cmath>
#include <type_traits>

namespace montecarlo {

/**
 * @brief Random number generator utilities for Monte Carlo simulations
 */
class RandomGenerator {
public:
    using Engine = std::mt19937_64;
    
    /**
     * @brief Construct a new Random Generator
     * 
     * @param seed Random seed (default: random device)
     */
    explicit RandomGenerator(unsigned int seed = std::random_device{}())
        : engine_(seed) {}
    
    /**
     * @brief Generate a uniform random number in [min, max]
     * 
     * @tparam T Number type (float, double, int, etc.)
     * @param min Minimum value
     * @param max Maximum value
     * @return T Random number
     */
    template<typename T>
    T uniform(T min, T max) {
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(engine_);
        } else {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(engine_);
        }
    }
    
    /**
     * @brief Generate a normal (Gaussian) random number
     * 
     * @tparam T Number type (float or double)
     * @param mean Mean of the distribution
     * @param stddev Standard deviation
     * @return T Random number
     */
    template<typename T = double>
    T normal(T mean = 0.0, T stddev = 1.0) {
        std::normal_distribution<T> dist(mean, stddev);
        return dist(engine_);
    }
    
    /**
     * @brief Generate an exponential random number
     * 
     * @tparam T Number type (float or double)
     * @param lambda Rate parameter
     * @return T Random number
     */
    template<typename T = double>
    T exponential(T lambda = 1.0) {
        std::exponential_distribution<T> dist(lambda);
        return dist(engine_);
    }
    
    /**
     * @brief Generate a Poisson random number
     * 
     * @param mean Mean of the distribution
     * @return int Random number
     */
    int poisson(double mean = 1.0) {
        std::poisson_distribution<int> dist(mean);
        return dist(engine_);
    }
    
    /**
     * @brief Generate a binomial random number
     * 
     * @param n Number of trials
     * @param p Probability of success
     * @return int Random number
     */
    int binomial(int n, double p) {
        std::binomial_distribution<int> dist(n, p);
        return dist(engine_);
    }
    
    /**
     * @brief Generate a gamma random number
     * 
     * @tparam T Number type (float or double)
     * @param alpha Shape parameter
     * @param beta Scale parameter (default: 1.0)
     * @return T Random number
     */
    template<typename T = double>
    T gamma(T alpha, T beta = 1.0) {
        std::gamma_distribution<T> dist(alpha, beta);
        return dist(engine_);
    }
    
    /**
     * @brief Generate a chi-squared random number
     * 
     * @tparam T Number type (float or double)
     * @param degrees_of_freedom Degrees of freedom
     * @return T Random number
     */
    template<typename T = double>
    T chiSquared(T degrees_of_freedom) {
        std::chi_squared_distribution<T> dist(degrees_of_freedom);
        return dist(engine_);
    }
    
    /**
     * @brief Generate a log-normal random number
     * 
     * @tparam T Number type (float or double)
     * @param mean Mean of underlying normal distribution
     * @param stddev Standard deviation of underlying normal distribution
     * @return T Random number
     */
    template<typename T = double>
    T logNormal(T mean = 0.0, T stddev = 1.0) {
        std::lognormal_distribution<T> dist(mean, stddev);
        return dist(engine_);
    }
    
    /**
     * @brief Seed the random number generator
     * 
     * @param seed New seed value
     */
    void seed(unsigned int seed) {
        engine_.seed(seed);
    }
    
    /**
     * @brief Get reference to the underlying engine
     */
    Engine& engine() { return engine_; }
    
    /**
     * @brief Get const reference to the underlying engine
     */
    const Engine& engine() const { return engine_; }
    
private:
    Engine engine_;
};

/**
 * @brief Thread-local random generator for parallel simulations
 */
inline RandomGenerator& getThreadLocalGenerator() {
    thread_local RandomGenerator generator(std::random_device{}());
    return generator;
}

/**
 * @brief Box-Muller transform for generating normal random variables
 * 
 * Generates a pair of independent standard normal random variables.
 * 
 * @tparam T Number type (float or double)
 * @param rng Random generator
 * @return std::pair<T, T> Two independent normal(0,1) random variables
 */
template<typename T = double>
std::pair<T, T> boxMuller(RandomGenerator& rng) {
    T u1 = rng.uniform<T>(0.0, 1.0);
    T u2 = rng.uniform<T>(0.0, 1.0);
    
    T r = std::sqrt(-2.0 * std::log(u1));
    T theta = 2.0 * M_PI * u2;
    
    return {r * std::cos(theta), r * std::sin(theta)};
}

} // namespace montecarlo
