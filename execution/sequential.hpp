#pragma once
#include <random>

namespace montecarlo::execution {

class Sequential {
 public:
    template<typename Model, typename Aggregator>
    void run(Model&& model, Aggregator& agg, size_t iterations, uint64_t seed = 42) {
        std::mt19937_64 rng(seed);

        for (size_t i = 0; i < iterations; ++i) {
            auto result = model(rng);
            agg.add(result);
        }
    }
};

} // namespace montecarlo::execution
