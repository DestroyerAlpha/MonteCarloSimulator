#include <montecarlo/montecarlo.h>
#include <iostream>
#include <iomanip>
#include <cmath>

/**
 * @brief Numerical integration using Monte Carlo method
 * 
 * This example demonstrates using Monte Carlo simulation to estimate
 * definite integrals.
 */

// Function to integrate: f(x) = x^2
double f1(double x) {
    return x * x;
}

// Function to integrate: f(x) = sin(x)
double f2(double x) {
    return std::sin(x);
}

// Function to integrate: f(x) = e^(-x^2) (Gaussian)
double f3(double x) {
    return std::exp(-x * x);
}

template<typename Func>
double monteCarloIntegrate(Func f, double a, double b, size_t num_samples) {
    using namespace montecarlo;
    
    MonteCarloSimulator<double> simulator(num_samples);
    
    auto integration_func = [&f, a, b]() -> double {
        auto& rng = getThreadLocalGenerator();
        double x = rng.uniform(a, b);
        return f(x);
    };
    
    auto result = simulator.run(integration_func);
    
    // The integral estimate is: (b - a) * mean
    return (b - a) * result.mean;
}

int main() {
    using namespace montecarlo;
    
    std::cout << "Monte Carlo Numerical Integration\n";
    std::cout << "==================================\n\n";
    
    const size_t num_samples = 100000;
    
    // Example 1: Integrate x^2 from 0 to 1
    // Analytical result: 1/3 ≈ 0.333333
    {
        double a = 0.0, b = 1.0;
        double estimate = monteCarloIntegrate(f1, a, b, num_samples);
        double actual = 1.0 / 3.0;
        
        std::cout << "Example 1: ∫₀¹ x² dx\n";
        std::cout << "  Estimated: " << std::fixed << std::setprecision(6) 
                  << estimate << "\n";
        std::cout << "  Actual:    " << actual << "\n";
        std::cout << "  Error:     " << std::abs(estimate - actual) << "\n\n";
    }
    
    // Example 2: Integrate sin(x) from 0 to π
    // Analytical result: 2
    {
        constexpr double PI = montecarlo::constants::PI;
        double a = 0.0, b = PI;
        double estimate = monteCarloIntegrate(f2, a, b, num_samples);
        double actual = 2.0;
        
        std::cout << "Example 2: ∫₀^π sin(x) dx\n";
        std::cout << "  Estimated: " << std::fixed << std::setprecision(6) 
                  << estimate << "\n";
        std::cout << "  Actual:    " << actual << "\n";
        std::cout << "  Error:     " << std::abs(estimate - actual) << "\n\n";
    }
    
    // Example 3: Integrate e^(-x^2) from 0 to 1
    // Analytical result: ≈ 0.746824
    {
        double a = 0.0, b = 1.0;
        double estimate = monteCarloIntegrate(f3, a, b, num_samples);
        double actual = 0.746824;  // Approximation
        
        std::cout << "Example 3: ∫₀¹ e^(-x²) dx\n";
        std::cout << "  Estimated: " << std::fixed << std::setprecision(6) 
                  << estimate << "\n";
        std::cout << "  Actual:    " << actual << " (approx)\n";
        std::cout << "  Error:     " << std::abs(estimate - actual) << "\n\n";
    }
    
    std::cout << "All integrations used " << num_samples << " samples.\n";
    
    return 0;
}
