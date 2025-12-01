#pragma once
#include <random>
#include <vector>
#include <thread>
#include <algorithm>
#include "../core/rng.hpp"

namespace montecarlo::execution {
class Parallel {
 public:
    explicit Parallel(size_t num_threads = 0) 
        : num_threads_(num_threads > 0 ? num_threads : std::thread::hardware_concurrency()) {}

    template<typename Model, typename Aggregator, typename RngFactory = ::montecarlo::DefaultRngFactory>
    void run(Model model, Aggregator& agg, size_t iterations, uint64_t seed = 42, RngFactory rng_factory = RngFactory{}) const {
        std::vector<std::thread> threads;
        std::vector<Aggregator> local_aggs(num_threads_);

        size_t iters_per_thread = iterations / num_threads_;
        size_t remaining = iterations % num_threads_;

        for (size_t t = 0; t < num_threads_; ++t) {
            size_t thread_iters = iters_per_thread + (t < remaining ? 1 : 0);
            threads.emplace_back([model, &local_aggs, t, thread_iters, seed, rng_factory]() mutable {
                auto rng = rng_factory(seed + static_cast<uint64_t>(t));
                for (std::uint64_t i = 0; i < thread_iters; ++i) {
                    double result;
                    if constexpr (requires { model.trial(rng); }) {
                        result = model.trial(rng);
                    } else {
                        result = model(rng);
                    }
                    local_aggs[t].add(result);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Merge results - aggregate all local results into main aggregator.
        // Prefer a native merge() if the aggregator exposes one, otherwise
        // fall back to replaying the per-thread means.
        agg.reset();
        for (const auto& local_agg : local_aggs) {
            if constexpr (requires { agg.merge(local_agg); }) {
                agg.merge(local_agg);
            } else {
                if (local_agg.count() > 0) {
                    for (size_t i = 0; i < local_agg.count(); ++i) {
                        agg.add(local_agg.result());
                    }
                }
            }
        }
    }

 private:
    size_t num_threads_;
};

} // namespace montecarlo::execution
