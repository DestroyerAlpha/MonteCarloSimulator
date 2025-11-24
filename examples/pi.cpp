#include "mc/core.hpp"

#include <iostream>
#include <random>

using namespace mc;

struct PiModel {
    template <typename Rng>
    double trial(Rng& rng) const {
        std::uniform_real_distribution<double> U(0.0, 1.0);
        double x = U(rng);
        double y = U(rng);
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    }
};

int main(int argc, char** argv) {
    std::uint64_t N = 10'000'000;

    if (argc > 1) {
        N = std::stoull(argv[1]);
    }

    auto rng_factory = [](std::uint64_t base_seed) {
        return make_rng(base_seed, 0);
    };

    auto transform = [](double indicator) {
        return 4.0 * indicator;
    };

    PiModel model;
    mc::SimulationEngine engine{model, rng_factory, transform, 123456789ULL};

    auto result = engine.run_sequential(N);
    auto ci     = ci_95(result);

    std::cout << "Monte Carlo Pi estimation (single-threaded)\n";
    std::cout << "samples   = " << result.n << "\n";
    std::cout << "estimate  = " << result.estimate << "\n";
    std::cout << "stderr    = " << result.standard_error << "\n";
    std::cout << "95% CI    = [" << ci.lo << ", " << ci.hi << "]\n";
    std::cout << "time      = " << result.elapsed_ms << " ms\n";

    return 0;
}
