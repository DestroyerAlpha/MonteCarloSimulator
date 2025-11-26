#pragma once
#include "concepts.hpp"
#include "result.hpp"
#include "rng.hpp"
#include "transform.hpp"
#include "../execution/sequential.hpp"
#ifdef MCLIB_PARALLEL_ENABLED
#include "../execution/parallel.hpp"
#endif
#ifdef MCLIB_GPU_ENABLED
#include "../execution/gpu.hpp"
#endif
#include <memory>
#include <chrono>

namespace montecarlo {

/**
 * @brief Main simulation engine for Monte Carlo experiments
 * 
 * @tparam Model The simulation model (must satisfy SimulationModel concept)
 * @tparam Aggregator The result aggregator (default: WelfordAggregator for numerical stability)
 * @tparam ExecutionPolicy The execution strategy (Sequential, Parallel, or GPU)
 * @tparam Transform Post-processing transform applied to each trial result
 */
template<
    typename Model,
    typename Aggregator = WelfordAggregator<>,
    typename ExecutionPolicy = execution::Sequential,
    typename Transform = transform::Identity
>
class SimulationEngine {
 public:
    using ModelType = Model;
    using AggregatorType = Aggregator;
    using PolicyType = ExecutionPolicy;
    using TransformType = Transform;

    /**
     * @brief Construct a simulation engine
     * 
     * @param model The simulation model instance
     * @param policy The execution policy for running trials
     * @param transform Optional transform applied to trial results
     * @param base_seed Base seed for random number generation
     */
    explicit SimulationEngine(
        Model model,
        ExecutionPolicy policy = ExecutionPolicy{},
        Transform transform = Transform{},
        std::uint64_t base_seed = 123456789ULL) :
        model_(model),
        policy_(policy),
        transform_(transform),
        base_seed_(base_seed)
    {}

    /**
     * @brief Run the Monte Carlo simulation
     * 
     * @param n Number of trials to execute
     * @return Result containing estimate, variance, std error, and timing
     */
    Result run(std::uint64_t iterations) const {
        auto start = std::chrono::steady_clock::now();

        Aggregator agg;

        // Create wrapped model that applies transform
        auto wrapped_model = [this](auto& rng) {
            double raw_result = invoke_model(rng);
            return transform_(raw_result);
        };

        policy_.run(wrapped_model, agg, iterations, base_seed_);

        auto end = std::chrono::steady_clock::now();

        return construct_result(agg, iterations, start, end);
    }

    /**
     * @brief Run simulation with a specific seed (compatibility method)
     * 
     * @param n Number of trials
     * @param seed Custom seed for this run
     * @return Result of the simulation
     */
    Result simulate(std::uint64_t iterations, std::uint64_t seed) const {
        // Temporarily override seed
        auto old_seed = base_seed_;
        const_cast<SimulationEngine*>(this)->base_seed_ = seed;
        auto result = run(iterations);
        const_cast<SimulationEngine*>(this)->base_seed_ = old_seed;
        return result;
    }

    /**
     * @brief Get the base seed used by this engine
     */
    std::uint64_t seed() const noexcept {
        return base_seed_;
    }

    /**
     * @brief Set a new base seed
     */
    void set_seed(std::uint64_t seed) noexcept {
        base_seed_ = seed;
    }

 private:
    /**
     * @brief Invoke model (handles both .trial() and operator() styles)
     */
    double invoke_model(auto& rng) const {
        if constexpr (requires { model_.trial(rng); }) {
            return model_.trial(rng);
        } else {
            return model_(rng);
        }
    }

    /**
     * @brief Construct result from aggregated data
     */
    Result construct_result(
        const Aggregator& agg, std::uint64_t iterations,
        const std::chrono::steady_clock::time_point& start,
        const std::chrono::steady_clock::time_point& end) const {
        Result r;
        r.iterations = iterations;
        r.estimate = agg.result();
        r.variance = agg.variance();
        r.standard_error = agg.std_error();
        r.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
        return r;
    }

 private:
    Model model_;
    ExecutionPolicy policy_;
    Transform transform_;
    std::uint64_t base_seed_;
};

// ============================================================================
// Factory Functions
// ============================================================================
template<
    typename Model,
    typename Policy = execution::Sequential,
    typename Aggregator = WelfordAggregator<>,
    typename Transform = transform::Identity
>
auto make_engine(
    Model model,
    Policy policy = Policy{},
    std::uint64_t seed = 123456789ULL,
    Transform transform = Transform{}) {
    return SimulationEngine<Model, Aggregator, Policy, Transform>(
        model, policy, transform, seed);
}

template<typename Model, typename Transform = transform::Identity>
auto make_sequential_engine(Model model, std::uint64_t seed = 123456789ULL, Transform transform = Transform{}) {
    return make_engine(model, execution::Sequential{}, seed, transform);
}

template<typename Model, typename Transform = transform::Identity>
auto make_parallel_engine(Model model, size_t threads = 0, std::uint64_t seed = 123456789ULL, Transform transform = Transform{}) {
    return make_engine(model, execution::Parallel{threads}, seed, transform);
}
}  // namespace montecarlo
