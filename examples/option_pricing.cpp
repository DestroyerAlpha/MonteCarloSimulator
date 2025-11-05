#include <montecarlo/montecarlo.h>
#include <iostream>
#include <iomanip>
#include <cmath>

/**
 * @brief European Call Option Pricing using Monte Carlo simulation
 * 
 * This example demonstrates pricing a European call option using the
 * Black-Scholes model with Monte Carlo simulation.
 */

struct OptionParameters {
    double spot_price;      // Current stock price
    double strike_price;    // Strike price
    double risk_free_rate;  // Risk-free interest rate
    double volatility;      // Volatility (sigma)
    double time_to_maturity; // Time to maturity in years
};

double simulateStockPrice(const OptionParameters& params, montecarlo::RandomGenerator& rng) {
    // Simulate stock price at maturity using geometric Brownian motion
    // S(T) = S(0) * exp((r - 0.5*sigma^2)*T + sigma*sqrt(T)*Z)
    // where Z ~ N(0,1)
    
    double drift = (params.risk_free_rate - 0.5 * params.volatility * params.volatility) 
                   * params.time_to_maturity;
    double diffusion = params.volatility * std::sqrt(params.time_to_maturity) 
                      * rng.normal(0.0, 1.0);
    
    return params.spot_price * std::exp(drift + diffusion);
}

double calculatePayoff(double stock_price, double strike_price) {
    // Call option payoff: max(S - K, 0)
    return std::max(stock_price - strike_price, 0.0);
}

int main() {
    using namespace montecarlo;
    
    std::cout << "European Call Option Pricing\n";
    std::cout << "=============================\n\n";
    
    // Option parameters
    OptionParameters params;
    params.spot_price = 100.0;
    params.strike_price = 105.0;
    params.risk_free_rate = 0.05;  // 5%
    params.volatility = 0.20;      // 20%
    params.time_to_maturity = 1.0; // 1 year
    
    std::cout << "Option Parameters:\n";
    std::cout << "  Spot Price:      $" << params.spot_price << "\n";
    std::cout << "  Strike Price:    $" << params.strike_price << "\n";
    std::cout << "  Risk-free Rate:  " << (params.risk_free_rate * 100) << "%\n";
    std::cout << "  Volatility:      " << (params.volatility * 100) << "%\n";
    std::cout << "  Time to Maturity: " << params.time_to_maturity << " year(s)\n\n";
    
    const size_t num_simulations = 100000;
    MonteCarloSimulator<double> simulator(num_simulations);
    
    // Simulation function
    auto option_simulation = [&params]() -> double {
        auto& rng = getThreadLocalGenerator();
        double final_price = simulateStockPrice(params, rng);
        return calculatePayoff(final_price, params.strike_price);
    };
    
    std::cout << "Running " << num_simulations << " simulations...\n";
    
    auto result = simulator.run(option_simulation);
    
    // Discount the expected payoff to present value
    double option_price = std::exp(-params.risk_free_rate * params.time_to_maturity) 
                         * result.mean;
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nResults:\n";
    std::cout << "  Option Price:    $" << option_price << "\n";
    std::cout << "  Mean Payoff:     $" << result.mean << "\n";
    std::cout << "  Std Dev:         $" << result.std_dev << "\n";
    std::cout << "  95% CI:          [$" << result.confidence_interval_lower 
              << ", $" << result.confidence_interval_upper << "]\n";
    std::cout << "  Min Payoff:      $" << result.min << "\n";
    std::cout << "  Max Payoff:      $" << result.max << "\n";
    
    return 0;
}
