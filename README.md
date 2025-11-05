# Monte Carlo Simulator Library

A generic, modern C++ implementation of Monte Carlo simulation techniques that can be used as a library for any use case requiring stochastic modeling and numerical analysis.

## Features

- 🚀 **Generic Template-Based Design**: Works with any numeric type
- ⚡ **Parallel Execution**: Multi-threaded simulations for improved performance
- 📊 **Statistical Analysis**: Automatic computation of mean, variance, confidence intervals, etc.
- 🎲 **Rich Random Distributions**: Support for uniform, normal, exponential, Poisson, binomial, gamma, and more
- 🔧 **Header-Only Library**: Easy integration into any C++ project
- 🎯 **Modern C++17**: Utilizes modern C++ features for clean, efficient code
- 📦 **CMake Support**: Simple build and installation process

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher

## Installation

### Using CMake

```bash
# Clone the repository
git clone https://github.com/DestroyerAlpha/MonteCarloSimulator.git
cd MonteCarloSimulator

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest

# Run examples
./examples/pi_estimation
./examples/option_pricing
./examples/integration
./examples/dice_rolling

# Install (optional)
sudo cmake --install .
```

### Header-Only Integration

Simply copy the `include/montecarlo` directory to your project and include:

```cpp
#include <montecarlo/montecarlo.h>
```

## Quick Start

### Basic Example

```cpp
#include <montecarlo/montecarlo.h>
#include <iostream>

int main() {
    using namespace montecarlo;
    
    // Create simulator with 10,000 runs
    MonteCarloSimulator<double> sim(10000);
    
    // Define simulation function
    auto result = sim.run([]() {
        auto& rng = getThreadLocalGenerator();
        return rng.normal(0.0, 1.0);
    });
    
    std::cout << "Mean: " << result.mean << std::endl;
    std::cout << "Std Dev: " << result.std_dev << std::endl;
    std::cout << "95% CI: [" << result.confidence_interval_lower 
              << ", " << result.confidence_interval_upper << "]" << std::endl;
    
    return 0;
}
```

### Estimating Pi

```cpp
MonteCarloSimulator<double> sim(1000000);

auto pi_simulation = []() -> double {
    auto& rng = getThreadLocalGenerator();
    double x = rng.uniform(0.0, 1.0);
    double y = rng.uniform(0.0, 1.0);
    return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
};

auto result = sim.run(pi_simulation);
double pi_estimate = 4.0 * result.mean;
std::cout << "Pi ≈ " << pi_estimate << std::endl;
```

## API Reference

### MonteCarloSimulator

Main simulator class template.

```cpp
template<typename T = double>
class MonteCarloSimulator;
```

**Methods:**
- `run(SimulationFunction func, bool parallel = true)`: Run simulation and return statistics
- `runRaw(SimulationFunction func, bool parallel = true)`: Return raw simulation results
- `setNumThreads(size_t num)`: Set number of threads for parallel execution
- `setConfidenceLevel(double level)`: Set confidence level (e.g., 0.95 for 95%)

### RandomGenerator

Random number generator with various distributions.

**Methods:**
- `uniform<T>(T min, T max)`: Uniform distribution
- `normal<T>(T mean, T stddev)`: Normal (Gaussian) distribution
- `exponential<T>(T lambda)`: Exponential distribution
- `poisson(double mean)`: Poisson distribution
- `binomial(int n, double p)`: Binomial distribution
- `gamma<T>(T alpha, T beta)`: Gamma distribution
- `logNormal<T>(T mean, T stddev)`: Log-normal distribution

### SimulationResult

Contains statistical results from simulation.

**Fields:**
- `mean`: Average of all simulation results
- `std_dev`: Standard deviation
- `variance`: Variance
- `min`: Minimum value
- `max`: Maximum value
- `confidence_interval_lower`: Lower bound of confidence interval
- `confidence_interval_upper`: Upper bound of confidence interval
- `num_samples`: Number of samples

## Examples

The library includes several complete examples:

1. **Pi Estimation**: Classic Monte Carlo method to estimate π
2. **Option Pricing**: European call option pricing using Black-Scholes
3. **Numerical Integration**: Computing definite integrals
4. **Dice Rolling**: Probability analysis for various dice games

Run examples after building:

```bash
./examples/pi_estimation
./examples/option_pricing
./examples/integration
./examples/dice_rolling
```

## Use Cases

This library can be used for:

- **Financial Modeling**: Option pricing, risk analysis, portfolio optimization
- **Physics Simulations**: Particle transport, radiation modeling
- **Numerical Integration**: Estimating complex integrals
- **Game Theory**: Probability analysis, strategy optimization
- **Machine Learning**: Sampling, uncertainty quantification
- **Operations Research**: Queueing theory, inventory management
- **Engineering**: Reliability analysis, sensitivity studies

## Performance

The library uses:
- `std::mt19937_64` for high-quality pseudo-random number generation
- Multi-threading via `std::async` for parallel simulations
- Move semantics for efficient data handling
- Template specialization for optimal performance

## Testing

Run the test suite:

```bash
cd build
ctest
# or
./tests/test_simulator
./tests/test_random
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details.

## Author

Destroyer Alpha

## Acknowledgments

- Inspired by classical Monte Carlo methods
- Uses C++17 standard library facilities
- Designed for flexibility and ease of use