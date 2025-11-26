#pragma once
#include <random>
#include "../core/rng.hpp"

namespace montecarlo::execution {

class Sequential {
 public:
    template<typename Model, typename Aggregator, typename RngFactory = ::montecarlo::DefaultRngFactory>
    void run(Model&& model, Aggregator& agg, size_t iterations, uint64_t seed = 42, RngFactory rng_factory = RngFactory{}) const {
        auto rng = rng_factory(seed);
        
        for (std::uint64_t i = 0; i < iterations; ++i) {
            double result;
            if constexpr (requires { model.trial(rng); }) {
                result = model.trial(rng);
            } else {
                result = model(rng);
            }
            agg.add(result);
        }
    }
};

} // namespace montecarlo::execution