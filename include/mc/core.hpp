#pragma once

#include <concepts>
#include <cstdint>
#include <random>
#include <chrono>
#include <cmath>

namespace mc {

template <typename M, typename Rng>
concept TrialModel =
    requires(M m, Rng& rng) {
        { m.trial(rng) } -> std::convertible_to<double>;
    };

inline std::mt19937_64 make_rng(std::uint64_t seed,
                                std::uint64_t stream_id = 0) {
    std::seed_seq seq{
        static_cast<unsigned>(seed),
        static_cast<unsigned>(seed >> 32),
        static_cast<unsigned>(stream_id),
        static_cast<unsigned>(stream_id >> 32)
    };
    return std::mt19937_64(seq);
}

struct Result {
    double estimate{};
    double variance{};
    double standard_error{};
    std::uint64_t n{};
    double elapsed_ms{};
};

struct CI {
    double lo;
    double hi;
};

inline CI ci_95(const Result& r) {
    constexpr double z = 1.96;
    return { r.estimate - z * r.standard_error,
             r.estimate + z * r.standard_error };
}

template <typename M, typename RngFactory, typename Estimator>
requires std::invocable<RngFactory, std::uint64_t> &&
         std::invocable<Estimator, double> &&
         TrialModel<M, std::invoke_result_t<RngFactory, std::uint64_t>>
class SimulationEngine {
public:
    using Model     = M;
    using SeedType  = std::uint64_t;
    using CountType = std::uint64_t;

    SimulationEngine(Model model,
                     RngFactory rng_factory,
                     Estimator transform,
                     SeedType base_seed = 123456789ULL)
        : model_(model)
        , rng_factory_(rng_factory)
        , transform_(transform)
        , base_seed_(base_seed)
    {}

    [[nodiscard]] Result run_sequential(CountType n) const {
        auto start = std::chrono::steady_clock::now();

        auto rng = rng_factory_(base_seed_);

        double mean = 0.0;
        double m2   = 0.0;

        for (CountType i = 1; i <= n; ++i) {
            double x = transform_(model_.trial(rng));
            double delta = x - mean;
            mean += delta / static_cast<double>(i);
            m2   += delta * (x - mean);
        }

        auto end = std::chrono::steady_clock::now();
        Result r;
        r.n        = n;
        r.estimate = mean;
        r.variance = (n > 1)
            ? (m2 / static_cast<double>(n - 1))
            : 0.0;
        r.standard_error = (n > 0)
            ? std::sqrt(r.variance / static_cast<double>(n))
            : 0.0;
        r.elapsed_ms =
            std::chrono::duration<double, std::milli>(end - start).count();
        return r;
    }

private:
    Model      model_;
    RngFactory rng_factory_;
    Estimator  transform_;
    SeedType   base_seed_;
};

}
