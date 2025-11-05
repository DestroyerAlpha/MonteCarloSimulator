#pragma once

/**
 * @file montecarlo.h
 * @brief Main header file for the Monte Carlo Simulator library
 * 
 * This library provides a generic, modern C++ implementation of Monte Carlo
 * simulation techniques that can be used for various stochastic modeling
 * and numerical analysis tasks.
 * 
 * Features:
 * - Template-based generic simulator
 * - Parallel execution support
 * - Statistical analysis and confidence intervals
 * - Various random distribution generators
 * - Thread-safe random number generation
 * - Header-only library for easy integration
 * 
 * Example usage:
 * @code
 * #include <montecarlo/montecarlo.h>
 * 
 * using namespace montecarlo;
 * 
 * // Create simulator with 10000 runs
 * MonteCarloSimulator<double> sim(10000);
 * 
 * // Define simulation function
 * auto result = sim.run([]() {
 *     auto& rng = getThreadLocalGenerator();
 *     return rng.normal(0.0, 1.0);
 * });
 * 
 * std::cout << "Mean: " << result.mean << std::endl;
 * std::cout << "Std Dev: " << result.std_dev << std::endl;
 * @endcode
 * 
 * @author Monte Carlo Simulator Library
 * @version 1.0.0
 */

#include "simulator.h"
#include "random.h"

namespace montecarlo {
    /**
     * @brief Library version information
     */
    constexpr int VERSION_MAJOR = 1;
    constexpr int VERSION_MINOR = 0;
    constexpr int VERSION_PATCH = 0;
    
    /**
     * @brief Mathematical constants
     */
    namespace constants {
        constexpr double PI = 3.141592653589793238462643383279502884;
        constexpr double E = 2.718281828459045235360287471352662498;
    }
    
    /**
     * @brief Get library version as string
     */
    inline const char* getVersion() {
        return "1.0.0";
    }
}

