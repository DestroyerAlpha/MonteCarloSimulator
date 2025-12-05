#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <random>
#include "montecarlo/montecarlo.hpp"
#include "example_functions.hpp"

// Model for estimating Pi using the circle method
class PiEstimationModel {
 public:
    template<typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        double y = dist(rng);

        // Check if dart lands inside the quarter circle
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    }
};

void run_pi_estimation() {
    std::cout << "=== Pi Estimation using Monte Carlo ===" << std::endl;
    std::cout << "True value of Pi: " << M_PI << std::endl << std::endl;

    PiEstimationModel model;

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
        auto engine = montecarlo::make_engine(model, montecarlo::execution::Sequential{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::LinearScale(4.0, 0.0));
        auto result = engine.run(n);

        double error = std::abs(result.estimate - M_PI);

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
        auto engine = montecarlo::make_engine(model, montecarlo::execution::Parallel{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::LinearScale(4.0, 0.0));
        auto result = engine.run(n);

        double error = std::abs(result.estimate - M_PI);

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
