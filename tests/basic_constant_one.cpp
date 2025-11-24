#include "mc/core.hpp"

#include <iostream>
#include <cmath>

using namespace mc;

struct ConstantOneModel {
    template <typename Rng>
    double trial(Rng& rng) const {
        (void)rng;
        return 1.0;
    }
};

int main() {
    auto rng_factory = [](std::uint64_t base_seed) {
        return make_rng(base_seed, 0);
    };

    auto transform = [](double x) {
        return x;
    };

    ConstantOneModel model;
    mc::SimulationEngine engine{model, rng_factory, transform, 424242ULL};

    std::uint64_t N = 1'000'000;
    auto result = engine.run_sequential(N);

    std::cout << "mean=" << result.estimate
              << " var=" << result.variance
              << " stderr=" << result.standard_error << "\n";

    if (std::abs(result.estimate - 1.0) < 1e-3) {
        std::cout << "TEST PASS\n";
        return 0;
    } else {
        std::cout << "TEST FAIL\n";
        return 1;
    }
}
