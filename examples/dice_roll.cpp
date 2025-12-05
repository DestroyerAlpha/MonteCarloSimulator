#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <random>
#include "montecarlo/montecarlo.hpp"
#include "example_functions.hpp"

void run_dice_roll() {
    auto dice_roll = [](auto& rng) {
        std::uniform_int_distribution<int> dist(1, 6);
        return static_cast<double>(dist(rng));
    };
    std::vector<size_t> sample_sizes {1'000, 10'000, 100'000, 1'000'000, 10'000'000};
    std::cout << "\n=== Dice Roll Expectation Estimation ===" << std::endl;
    std::cout<< "Expectation value of a fair six-sided die is 3.5" << std::endl << std::endl;
    std::cout << "Sequential Execution:" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    std::cout << std::setw(12) << "Samples"
              << std::setw(15) << "Estimate"
              << std::setw(15) << "Error"
              << std::setw(15) << "Std Error"
              << std::setw(13) << "Time (ms)" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t n : sample_sizes) {
        auto engine = montecarlo::make_engine(dice_roll, montecarlo::execution::Sequential{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 3.5);

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
        auto engine = montecarlo::make_engine(dice_roll, montecarlo::execution::Parallel{}, 42ULL, montecarlo::DefaultRngFactory{}, montecarlo::transform::Identity{});
        auto result = engine.run(n);

        double error = std::abs(result.estimate - 3.5);

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
