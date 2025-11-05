#include <montecarlo/montecarlo.h>
#include <iostream>
#include <iomanip>

/**
 * @brief Estimate the value of Pi using Monte Carlo simulation
 * 
 * This example uses the classic Monte Carlo method to estimate Pi by
 * randomly sampling points in a square and checking if they fall within
 * a quarter circle.
 */

int main() {
    using namespace montecarlo;
    
    std::cout << "Monte Carlo Pi Estimation\n";
    std::cout << "==========================\n\n";
    
    const size_t num_simulations = 1000000;
    
    MonteCarloSimulator<double> simulator(num_simulations);
    
    // Simulation function: returns 1 if point is inside quarter circle, 0 otherwise
    auto pi_simulation = []() -> double {
        auto& rng = getThreadLocalGenerator();
        double x = rng.uniform(0.0, 1.0);
        double y = rng.uniform(0.0, 1.0);
        
        // Check if point (x, y) is inside quarter circle
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    };
    
    std::cout << "Running " << num_simulations << " simulations...\n";
    
    auto result = simulator.run(pi_simulation);
    
    // Pi estimation: 4 * (points inside circle / total points)
    double pi_estimate = 4.0 * result.mean;
    constexpr double PI = constants::PI;
    double pi_error = std::abs(pi_estimate - PI);
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\nResults:\n";
    std::cout << "  Estimated Pi: " << pi_estimate << "\n";
    std::cout << "  Actual Pi:    " << PI << "\n";
    std::cout << "  Error:        " << pi_error << "\n";
    std::cout << "  Error %:      " << (pi_error / PI * 100) << "%\n";
    std::cout << "\nStatistics:\n";
    std::cout << "  Mean ratio:   " << result.mean << "\n";
    std::cout << "  Std Dev:      " << result.std_dev << "\n";
    std::cout << "  95% CI:       [" << result.confidence_interval_lower 
              << ", " << result.confidence_interval_upper << "]\n";
    
    return 0;
}
