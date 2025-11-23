#pragma once
#include "concepts.hpp"
#include "result.hpp"
#include "../execution/sequential.hpp"
#include <memory>
#include <chrono>

namespace montecarlo {

template<typename Model, typename Aggregator = MeanAggregator<>, 
         typename ExecutionPolicy = execution::Sequential>
class MonteCarloEngine {
 public:
    MonteCarloEngine(Model model, ExecutionPolicy policy = ExecutionPolicy{})
        : model_(std::move(model)), policy_(std::move(policy)) {}

    struct SimulationResult {
        double value;
        double std_error;
        size_t iterations;
        double elapsed_seconds;
    };

    SimulationResult simulate(size_t iterations, uint64_t seed = 42) {
        Aggregator agg;

        auto start = std::chrono::high_resolution_clock::now();
        policy_.run(model_, agg, iterations, seed);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        return SimulationResult{
            agg.result(),
            agg.std_error(),
            iterations,
            elapsed.count()
        };
    }

    // Run with custom aggregator
    template<typename CustomAgg>
    auto simulate_with(CustomAgg& agg, size_t iterations, uint64_t seed = 42) {
        auto start = std::chrono::high_resolution_clock::now();
        policy_.run(model_, agg, iterations, seed);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        return elapsed.count();
    }

 private:
    Model model_;
    ExecutionPolicy policy_;
};

// Deduction guide
template<typename Model>
MonteCarloEngine(Model) -> MonteCarloEngine<Model, MeanAggregator<>, execution::Sequential>;

// Factory functions
template<typename Model>
auto make_sequential_engine(Model&& model) {
    return MonteCarloEngine<Model, MeanAggregator<>, execution::Sequential>(
        std::forward<Model>(model), execution::Sequential{});
}

template<typename Model>
auto make_parallel_engine(Model&& model, size_t num_threads = 0) {
    return MonteCarloEngine<Model, MeanAggregator<>, execution::Parallel>(
        std::forward<Model>(model), execution::Parallel{num_threads});
}

} // namespace montecarlo
