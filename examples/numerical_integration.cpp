#include "montecarlo/montecarlo.hpp"
#include "example_functions.hpp"
#include <iostream>
#include <iomanip>

// Model for integrating f(x) = x^2 from 0 to 1
// Analytical result: 1/3
class SquareIntegrationModel {
 public:
    template<typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        return x * x; // f(x) * (b-a), where (b-a) = 1
    }
};

// Model for integrating a multivariate function
// f(x,y,z) = x^2 + y^2 + z^2 over [0,1]^3
// Analytical result: 1
class MultivarIntegrationModel {
 public:
    template<typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        double y = dist(rng);
        double z = dist(rng);
        return x*x + y*y + z*z; // f(x,y,z) * volume, where volume = 1
    }
};

void run_numerical_integration() {
    std::cout << "\n=== Numerical Integration using Monte Carlo ===" << std::endl;

    std::cout << "\n1D Integration: ∫₀¹ x² dx" << std::endl;
    std::cout << "Analytical result: " << 1.0/3.0 << std::endl << std::endl;
    std::vector<size_t> sample_sizes {1'000, 10'000, 100'000, 1'000'000, 10'000'000};

    SquareIntegrationModel model1d;
    std::cout << "Sequential Execution:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(12) << "Samples"
              << std::setw(15) << "Estimate"
              << std::setw(15) << "Error"
              << std::setw(15) << "Std Error"
              << std::setw(13) << "Time (ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t n : sample_sizes) {
        auto engine = montecarlo::make_engine(model1d, montecarlo::execution::Sequential{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 1.0/3.0);

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
        auto engine = montecarlo::make_engine(model1d, montecarlo::execution::Parallel{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 1.0/3.0);

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
    std::cout << "\n3D Integration: ∫∫∫ (x² + y² + z²) dx dy dz over [0,1]³" << std::endl;
    std::cout << "Analytical result: 1.0" << std::endl << std::endl;

    MultivarIntegrationModel model3d;
    std::cout << "Sequential Execution:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(12) << "Samples"
              << std::setw(15) << "Estimate"
              << std::setw(15) << "Error"
              << std::setw(15) << "Std Error"
              << std::setw(13) << "Time (ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t n : sample_sizes) {
        auto engine = montecarlo::make_engine(model3d, montecarlo::execution::Sequential{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 1.0);

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
        auto engine = montecarlo::make_engine(model3d, montecarlo::execution::Parallel{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 1.0);

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
