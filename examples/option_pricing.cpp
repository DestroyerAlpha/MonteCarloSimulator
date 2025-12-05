#include "montecarlo/montecarlo.hpp"
#include "example_functions.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

// Black-Scholes European Call Option pricing model
class EuropeanCallOption {
 public:
    EuropeanCallOption(double S0, double K, double r, double sigma, double T)
        : S0_(S0), K_(K), r_(r), sigma_(sigma), T_(T) {}

    template<typename RNG>
    double operator()(RNG& rng) const {
        std::normal_distribution<double> dist(0.0, 1.0);
        double Z = dist(rng);

        // Terminal stock price using geometric Brownian motion
        double ST = S0_ * std::exp((r_ - 0.5 * sigma_ * sigma_) * T_ + sigma_ * std::sqrt(T_) * Z);

        // Payoff of call option
        double payoff = std::max(ST - K_, 0.0);

        // Discounted payoff
        return std::exp(-r_ * T_) * payoff;
    }

    // Analytical Black-Scholes formula for comparison
    double analytical_price() const {
        double d1 = (std::log(S0_ / K_) + (r_ + 0.5 * sigma_ * sigma_) * T_) / (sigma_ * std::sqrt(T_));
        double d2 = d1 - sigma_ * std::sqrt(T_);

        auto normal_cdf = [](double x) {
            return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
        };

        return S0_ * normal_cdf(d1) - K_ * std::exp(-r_ * T_) * normal_cdf(d2);
    }

 private:
    double S0_;     // Current stock price
    double K_;      // Strike price
    double r_;      // Risk-free rate
    double sigma_;  // Volatility
    double T_;      // Time to maturity
};

void run_option_pricing() {
    EuropeanCallOption model(100.0, 100.0, 0.05, 0.20, 1.0);
    double analytical = model.analytical_price();
    std::cout << "\n=== European Call Option Pricing ===" << std::endl;
    std::cout << "True Price using Black-Scholes Model: " << analytical << std::endl;
    // Parameters: S0=100, K=100, r=5%, sigma=20%, T=1 year
    std::vector<size_t> sample_sizes {1'000, 10'000, 100'000, 1'000'000, 10'000'000};

    std::cout << "Sequential Execution:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(12) << "Samples"
              << std::setw(15) << "Estimate"
              << std::setw(15) << "Error"
              << std::setw(15) << "Std Error"
              << std::setw(13) << "Time (ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t n : sample_sizes) {
        auto engine = montecarlo::make_engine(model, montecarlo::execution::Sequential{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - analytical);

        std::cout << std::setw(12) << result.iterations
        << std::setw(15) << std::fixed << std::setprecision(6) << result.estimate
        << std::setw(15) << std::scientific << std::setprecision(2) << error
        << std::setw(15) << std::fixed << std::setprecision(6)
        << result.standard_error << std::setw(13) << std::fixed
        << std::setprecision(4) << result.elapsed_ms << std::endl;
    }

#ifdef MCLIB_PARALLEL_ENABLED
    std::cout << "\nParallel Execution:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(12) << "Samples"
              << std::setw(15) << "Estimate"
              << std::setw(15) << "Error"
              << std::setw(15) << "Std Error"
              << std::setw(13) << "Time (ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t n : sample_sizes) {
        auto engine = montecarlo::make_engine(model, montecarlo::execution::Parallel{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - analytical);

        std::cout << std::setw(12) << result.iterations
        << std::setw(15) << std::fixed << std::setprecision(6) << result.estimate
        << std::setw(15) << std::scientific << std::setprecision(2) << error
        << std::setw(15) << std::fixed << std::setprecision(6)
        << result.standard_error << std::setw(13) << std::fixed
        << std::setprecision(4) << result.elapsed_ms << std::endl;
    }
#else
    std::cout << std::endl
    << "Parallel execution not enabled."
    << "Rebuild with -DMCLIB_ENABLE_PARALLEL=ON"
    << std::endl;
#endif
}
