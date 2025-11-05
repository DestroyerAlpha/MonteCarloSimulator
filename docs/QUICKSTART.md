# Quick Start Guide

## Installation

### Option 1: Build and Install

```bash
git clone https://github.com/DestroyerAlpha/MonteCarloSimulator.git
cd MonteCarloSimulator
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Option 2: Header-Only Integration

Copy the `include/montecarlo` directory to your project:

```bash
cp -r include/montecarlo /path/to/your/project/include/
```

## Your First Simulation

Create a file `my_simulation.cpp`:

```cpp
#include <montecarlo/montecarlo.h>
#include <iostream>

int main() {
    using namespace montecarlo;
    
    // Create a simulator with 10,000 runs
    MonteCarloSimulator<double> simulator(10000);
    
    // Run a simple simulation - rolling a die
    auto result = simulator.run([]() {
        auto& rng = getThreadLocalGenerator();
        return static_cast<double>(rng.uniform(1, 6));
    });
    
    std::cout << "Average die roll: " << result.mean << std::endl;
    std::cout << "Standard deviation: " << result.std_dev << std::endl;
    std::cout << "95% confidence interval: [" 
              << result.confidence_interval_lower << ", " 
              << result.confidence_interval_upper << "]" << std::endl;
    
    return 0;
}
```

Compile and run:

```bash
# If installed system-wide
g++ -std=c++17 my_simulation.cpp -o my_simulation -pthread
./my_simulation

# If using header-only
g++ -std=c++17 -I/path/to/include my_simulation.cpp -o my_simulation -pthread
./my_simulation
```

Or with CMake:

```cmake
cmake_minimum_required(VERSION 3.14)
project(MySimulation)

set(CMAKE_CXX_STANDARD 17)

# Option 1: If MonteCarloSimulator is installed
find_package(MonteCarloSimulator REQUIRED)
add_executable(my_simulation my_simulation.cpp)
target_link_libraries(my_simulation PRIVATE MonteCarloSimulator::montecarlo)

# Option 2: If using as subdirectory
# add_subdirectory(path/to/MonteCarloSimulator)
# add_executable(my_simulation my_simulation.cpp)
# target_link_libraries(my_simulation PRIVATE montecarlo)
```

## Common Patterns

### 1. Estimating Probabilities

```cpp
MonteCarloSimulator<double> sim(100000);

// Probability of event
auto result = sim.run([]() {
    auto& rng = getThreadLocalGenerator();
    // Return 1.0 if event occurs, 0.0 otherwise
    double x = rng.uniform(0.0, 1.0);
    return (x > 0.7) ? 1.0 : 0.0;
});

double probability = result.mean;  // Should be ~0.3
```

### 2. Computing Expected Values

```cpp
MonteCarloSimulator<double> sim(100000);

// Expected value of some random process
auto result = sim.run([]() {
    auto& rng = getThreadLocalGenerator();
    double x = rng.normal(10.0, 2.0);
    return x * x;  // E[X²]
});

double expected_value = result.mean;
```

### 3. Numerical Integration

```cpp
// Integrate f(x) = x² from 0 to 1
MonteCarloSimulator<double> sim(100000);

auto result = sim.run([]() {
    auto& rng = getThreadLocalGenerator();
    double x = rng.uniform(0.0, 1.0);
    return x * x;  // Function to integrate
});

double integral = (1.0 - 0.0) * result.mean;  // (b-a) * mean
// Should be approximately 1/3 ≈ 0.333
```

### 4. Sensitivity Analysis

```cpp
void analyze_parameter(double param) {
    MonteCarloSimulator<double> sim(10000);
    
    auto result = sim.run([param]() {
        auto& rng = getThreadLocalGenerator();
        return simulate_with_parameter(param, rng);
    });
    
    std::cout << "Parameter: " << param 
              << ", Mean: " << result.mean 
              << ", StdDev: " << result.std_dev << std::endl;
}

// Sweep through parameter values
for (double p = 0.1; p <= 1.0; p += 0.1) {
    analyze_parameter(p);
}
```

### 5. Using Different Distributions

```cpp
MonteCarloSimulator<double> sim(10000);

auto result = sim.run([]() {
    auto& rng = getThreadLocalGenerator();
    
    // Choose based on your needs:
    // return rng.uniform(0.0, 10.0);        // Uniform [0, 10]
    // return rng.normal(5.0, 1.5);          // Normal(5, 1.5)
    // return rng.exponential(2.0);          // Exponential(λ=2)
    // return rng.poisson(3.0);              // Poisson(λ=3)
    // return rng.binomial(10, 0.5);         // Binomial(n=10, p=0.5)
    // return rng.gamma(2.0, 2.0);           // Gamma(α=2, β=2)
    // return rng.logNormal(0.0, 1.0);       // LogNormal(μ=0, σ=1)
    
    return rng.normal(0.0, 1.0);
});
```

## Advanced Features

### Parallel Execution Control

```cpp
MonteCarloSimulator<double> sim(1000000);

// Set number of threads
sim.setNumThreads(4);  // Use 4 threads

// Run in parallel (default)
auto result1 = sim.run(my_function, true);

// Run sequentially
auto result2 = sim.run(my_function, false);
```

### Custom Confidence Levels

```cpp
MonteCarloSimulator<double> sim(10000);

// Set 99% confidence level
sim.setConfidenceLevel(0.99);

auto result = sim.run(my_function);
// result.confidence_interval_lower and _upper are now 99% CI
```

### Getting Raw Results

```cpp
MonteCarloSimulator<double> sim(1000);

// Get all simulation results
std::vector<double> results = sim.runRaw(my_function);

// Do custom analysis
auto median = calculate_median(results);
auto percentile_95 = calculate_percentile(results, 0.95);
```

## Tips

1. **Start small**: Test with fewer simulations first (1,000-10,000)
2. **Check convergence**: Run multiple times with increasing sample sizes
3. **Use parallel execution**: For computationally intensive simulations
4. **Set fixed seeds for testing**: Use `RandomGenerator(fixed_seed)` for reproducibility
5. **Validate results**: Compare with analytical solutions when available

## Next Steps

- Explore the [examples](../examples/) directory for more complex use cases
- Read the [Developer Guide](../DEVELOPER_GUIDE.md) for in-depth information
- Check the [README](../README.md) for API reference
