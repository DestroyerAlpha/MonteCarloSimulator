#pragma once
#include <concepts>
#include <random>
#include <type_traits>
#include <cstdint>

namespace montecarlo {

template<typename Model, typename RNG>
concept TrialModel = requires(Model m, RNG& rng) {
    { m.trial(rng) } -> std::convertible_to<double>;
};

template<typename Model, typename RNG>
concept CallableModel = requires(Model m, RNG& rng) {
    { m(rng) } -> std::convertible_to<double>;
};

template<typename Model, typename RNG>
concept SimulationModel = TrialModel<Model, RNG> || CallableModel<Model, RNG>;

template<typename T>
concept Transform = requires(T transform, double value) {
    { transform(value) } -> std::convertible_to<double>;
};

template<typename Aggregator, typename T>
concept ResultAggregator = requires(Aggregator agg, T value) {
    { agg.add(value) } -> std::same_as<void>;
    { agg.result() } -> std::convertible_to<double>;
    { agg.reset() } -> std::same_as<void>;
};

// RNG factory concept: callable with a seed returning a UniformRandomBitGenerator
template<typename F>
concept RngFactory = requires(F f, std::uint64_t s) {
    { f(s) };
} && std::uniform_random_bit_generator<std::decay_t<decltype(std::declval<F>()(0u))>>;
}  // namespace montecarlo
