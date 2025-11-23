#pragma once
#include <concepts>
#include <random>

namespace montecarlo {

// Concept: A simulation model must be callable with a random engine
template<typename Model, typename RNG>
concept SimulationModel = requires(Model m, RNG& rng) {
    { m(rng) } -> std::convertible_to<double>;
};

// Concept: A result aggregator must accumulate results
template<typename Aggregator, typename T>
concept ResultAggregator = requires(Aggregator agg, T value) {
    { agg.add(value) } -> std::same_as<void>;
    { agg.result() } -> std::convertible_to<double>;
    { agg.reset() } -> std::same_as<void>;
};

} // namespace montecarlo
