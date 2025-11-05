#pragma once

#include <random>
#include <vector>
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <thread>
#include <future>
#include <type_traits>

namespace montecarlo {

/**
 * @brief Statistics result from Monte Carlo simulation
 */
template<typename T = double>
struct SimulationResult {
    T mean;
    T std_dev;
    T variance;
    T min;
    T max;
    T confidence_interval_lower;
    T confidence_interval_upper;
    size_t num_samples;
    
    SimulationResult() : mean(0), std_dev(0), variance(0), 
                        min(0), max(0),
                        confidence_interval_lower(0),
                        confidence_interval_upper(0),
                        num_samples(0) {}
};

/**
 * @brief Generic Monte Carlo Simulator
 * 
 * A flexible template-based Monte Carlo simulator that can be used for various
 * stochastic simulations. The simulator supports:
 * - Custom simulation functions
 * - Parallel execution
 * - Statistical analysis of results
 * - Confidence intervals
 * 
 * @tparam T The data type for simulation results (default: double)
 */
template<typename T = double>
class MonteCarloSimulator {
public:
    using SimulationFunction = std::function<T()>;
    using RandomEngine = std::mt19937_64;
    
    /**
     * @brief Construct a new Monte Carlo Simulator
     * 
     * @param num_simulations Number of simulation runs
     * @param seed Random seed (default: random device)
     */
    explicit MonteCarloSimulator(size_t num_simulations, 
                                unsigned int seed = std::random_device{}())
        : num_simulations_(num_simulations)
        , seed_(seed)
        , confidence_level_(0.95)
        , num_threads_(std::thread::hardware_concurrency()) {
        if (num_threads_ == 0) num_threads_ = 1;
    }
    
    /**
     * @brief Set the number of threads for parallel execution
     * 
     * @param num_threads Number of threads (0 = auto-detect)
     */
    void setNumThreads(size_t num_threads) {
        if (num_threads == 0) {
            num_threads_ = std::thread::hardware_concurrency();
            if (num_threads_ == 0) num_threads_ = 1;
        } else {
            num_threads_ = num_threads;
        }
    }
    
    /**
     * @brief Set confidence level for confidence intervals
     * 
     * @param level Confidence level (e.g., 0.95 for 95%)
     */
    void setConfidenceLevel(double level) {
        if (level > 0.0 && level < 1.0) {
            confidence_level_ = level;
        }
    }
    
    /**
     * @brief Run the Monte Carlo simulation
     * 
     * @param simulation_func Function to execute for each simulation
     * @param parallel Enable parallel execution (default: true)
     * @return SimulationResult<T> Statistical results
     */
    SimulationResult<T> run(SimulationFunction simulation_func, bool parallel = true) {
        std::vector<T> results;
        results.reserve(num_simulations_);
        
        if (parallel && num_threads_ > 1) {
            results = runParallel(simulation_func);
        } else {
            results = runSequential(simulation_func);
        }
        
        return computeStatistics(results);
    }
    
    /**
     * @brief Run simulation and return all raw results
     * 
     * @param simulation_func Function to execute for each simulation
     * @param parallel Enable parallel execution (default: true)
     * @return std::vector<T> All simulation results
     */
    std::vector<T> runRaw(SimulationFunction simulation_func, bool parallel = true) {
        if (parallel && num_threads_ > 1) {
            return runParallel(simulation_func);
        } else {
            return runSequential(simulation_func);
        }
    }
    
    /**
     * @brief Get the number of simulations
     */
    size_t getNumSimulations() const { return num_simulations_; }
    
    /**
     * @brief Get the random seed
     */
    unsigned int getSeed() const { return seed_; }
    
private:
    std::vector<T> runSequential(SimulationFunction simulation_func) {
        std::vector<T> results;
        results.reserve(num_simulations_);
        
        for (size_t i = 0; i < num_simulations_; ++i) {
            results.push_back(simulation_func());
        }
        
        return results;
    }
    
    std::vector<T> runParallel(SimulationFunction simulation_func) {
        std::vector<T> results;
        results.reserve(num_simulations_);
        
        size_t sims_per_thread = num_simulations_ / num_threads_;
        size_t remaining = num_simulations_ % num_threads_;
        
        std::vector<std::future<std::vector<T>>> futures;
        futures.reserve(num_threads_);
        
        size_t offset = 0;
        for (size_t i = 0; i < num_threads_; ++i) {
            size_t count = sims_per_thread + (i < remaining ? 1 : 0);
            
            futures.push_back(std::async(std::launch::async, 
                [simulation_func, count]() {
                    std::vector<T> thread_results;
                    thread_results.reserve(count);
                    for (size_t j = 0; j < count; ++j) {
                        thread_results.push_back(simulation_func());
                    }
                    return thread_results;
                }));
            
            offset += count;
        }
        
        for (auto& future : futures) {
            auto thread_results = future.get();
            results.insert(results.end(), 
                          std::make_move_iterator(thread_results.begin()),
                          std::make_move_iterator(thread_results.end()));
        }
        
        return results;
    }
    
    SimulationResult<T> computeStatistics(const std::vector<T>& results) {
        SimulationResult<T> stats;
        stats.num_samples = results.size();
        
        if (results.empty()) {
            return stats;
        }
        
        // Mean
        stats.mean = std::accumulate(results.begin(), results.end(), T(0)) 
                    / static_cast<T>(results.size());
        
        // Variance and standard deviation
        T sum_squared_diff = std::accumulate(results.begin(), results.end(), T(0),
            [&stats](T acc, T val) {
                T diff = val - stats.mean;
                return acc + diff * diff;
            });
        
        stats.variance = sum_squared_diff / static_cast<T>(results.size());
        stats.std_dev = std::sqrt(stats.variance);
        
        // Min and max
        auto minmax = std::minmax_element(results.begin(), results.end());
        stats.min = *minmax.first;
        stats.max = *minmax.second;
        
        // Confidence interval (using normal approximation)
        // For 95% confidence: z = 1.96, for 99%: z = 2.576
        double z_score = getZScore(confidence_level_);
        T margin = static_cast<T>(z_score) * stats.std_dev / 
                   std::sqrt(static_cast<T>(results.size()));
        stats.confidence_interval_lower = stats.mean - margin;
        stats.confidence_interval_upper = stats.mean + margin;
        
        return stats;
    }
    
    double getZScore(double confidence_level) const {
        // Approximate z-scores for common confidence levels
        if (confidence_level >= 0.99) return 2.576;
        if (confidence_level >= 0.95) return 1.96;
        if (confidence_level >= 0.90) return 1.645;
        if (confidence_level >= 0.80) return 1.282;
        return 1.96; // Default to 95%
    }
    
    size_t num_simulations_;
    unsigned int seed_;
    double confidence_level_;
    size_t num_threads_;
};

} // namespace montecarlo
