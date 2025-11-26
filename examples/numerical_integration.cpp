#include "montecarlo/montecarlo.hpp"
#include "example_functions.hpp"
#include <iostream>
#include <iomanip>

// Model for integrating f(x) = x^2 from 0 to 1
// Analytical result: 1/3
class SquareIntegrationModel {
 public:
    template<typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        return x * x; // f(x) * (b-a), where (b-a) = 1
    }
};

// Model for integrating a multivariate function
// f(x,y,z) = x^2 + y^2 + z^2 over [0,1]^3
// Analytical result: 1
class MultivarIntegrationModel {
 public:
    template<typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        double y = dist(rng);
        double z = dist(rng);
        return x*x + y*y + z*z; // f(x,y,z) * volume, where volume = 1
    }
};

void run_numerical_integration() {
    std::cout << "\n=== Numerical Integration using Monte Carlo ===" << std::endl;

    std::cout << "\n1D Integration: ∫₀¹ x² dx" << std::endl;
    std::cout << "Analytical result: " << 1.0/3.0 << std::endl << std::endl;

    SquareIntegrationModel model1d;
#ifdef MCLIB_PARALLEL_ENABLED
    auto engine1d = montecarlo::make_parallel_engine(model1d);
#else
    auto engine1d = montecarlo::make_sequential_engine(model1d);
#endif

    for (size_t n : {1'000, 10'000, 100'000, 1'000'000, 10'000'000}) {
        auto result = engine1d.run(n);
        double error = std::abs(result.estimate - 1.0/3.0);

        std::cout << "N = " << std::setw(7) << n 
                  << ", Estimate = " << std::fixed << std::setprecision(6) << result.estimate
                  << ", Error = " << std::scientific << std::setprecision(2) << error
                  << ", Time (ms)= " << std::fixed << std::setprecision(4) << result.elapsed_ms << "s"
                  << std::endl;
    }

    std::cout << "\n3D Integration: ∫∫∫ (x² + y² + z²) dx dy dz over [0,1]³" << std::endl;
    std::cout << "Analytical result: 1.0" << std::endl << std::endl;

    MultivarIntegrationModel model3d;
#ifdef MCLIB_PARALLEL_ENABLED
    auto engine3d = montecarlo::make_parallel_engine(model3d);
#else
    auto engine3d = montecarlo::make_sequential_engine(model3d);
#endif

    for (size_t n : {1'000, 10'000, 100'000, 1'000'000, 10'000'000}) {
        auto result = engine3d.run(n);
        double error = std::abs(result.estimate - 1.0);

        std::cout << "N = " << std::setw(7) << n 
                  << ", Estimate = " << std::fixed << std::setprecision(6) << result.estimate
                  << ", Error = " << std::scientific << std::setprecision(2) << error
                  << ", Time (ms)= " << std::fixed << std::setprecision(4) << result.elapsed_ms << "s"
                  << std::endl;
    }
}
