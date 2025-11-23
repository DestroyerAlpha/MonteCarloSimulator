#pragma once
#include <random>
#include <vector>
#include <thread>
#include <mutex>

namespace montecarlo::execution {

class Parallel {
 public:
    explicit Parallel(size_t num_threads = 0) 
        : num_threads_(num_threads > 0 ? num_threads : std::thread::hardware_concurrency()) {}

    template<typename Model, typename Aggregator>
    void run(Model&& model, Aggregator& agg, size_t iterations, uint64_t seed = 42) {
        std::vector<std::thread> threads;
        std::vector<Aggregator> local_aggs(num_threads_);

        size_t iters_per_thread = iterations / num_threads_;
        size_t remaining = iterations % num_threads_;

        for (size_t t = 0; t < num_threads_; ++t) {
            size_t thread_iters = iters_per_thread + (t < remaining ? 1 : 0);
            threads.emplace_back([&model, &local_aggs, t, thread_iters, seed]() {
                std::mt19937_64 rng(seed + t);
                for (size_t i = 0; i < thread_iters; ++i) {
                    auto result = model(rng);
                    local_aggs[t].add(result);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Merge results
        agg.reset();
        for (const auto& local_agg : local_aggs) {
            for (size_t i = 0; i < local_agg.count(); ++i) {
                // Note: This is simplified. For actual merging, you'd need
                // the aggregator to expose its raw data or provide merge()
            }
        }

        // Simplified: just add the mean results
        for (const auto& local_agg : local_aggs) {
            for (size_t i = 0; i < local_agg.count(); ++i) {
                agg.add(local_agg.result());
            }
        }
    }

 private:
    size_t num_threads_;
};

} // namespace montecarlo::execution
