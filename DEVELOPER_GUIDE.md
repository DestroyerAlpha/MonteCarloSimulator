# Monte Carlo Simulator - Developer Guide

## Architecture

The library is designed as a header-only template library with the following components:

### Core Components

1. **MonteCarloSimulator** (`simulator.h`)
   - Main simulation engine
   - Template-based for type flexibility
   - Supports both sequential and parallel execution
   - Automatic statistical analysis

2. **RandomGenerator** (`random.h`)
   - Wraps C++ standard library random number generators
   - Provides convenient interfaces for common distributions
   - Thread-local generators for parallel execution

3. **SimulationResult** (`simulator.h`)
   - Contains statistical results
   - Mean, variance, standard deviation
   - Confidence intervals
   - Min/max values

## Design Decisions

### Header-Only Library
- **Pros**: Easy integration, no linking required, compiler optimizations
- **Cons**: Longer compilation times for large projects
- **Rationale**: Simplicity and flexibility for template-heavy code

### Template-Based Design
- Allows usage with any numeric type (int, float, double, custom types)
- Enables compiler optimizations through template specialization
- Type-safe at compile time

### Parallel Execution
- Uses `std::async` with `std::launch::async`
- Thread pool approach with balanced workload distribution
- Thread-local random generators to avoid contention

### Statistical Analysis
- Normal approximation for confidence intervals
- Variance calculated in single pass
- Numerically stable algorithms

## Extending the Library

### Adding New Distribution Functions

To add a new distribution to `RandomGenerator`:

```cpp
template<typename T = double>
T myDistribution(T param1, T param2) {
    std::my_distribution<T> dist(param1, param2);
    return dist(engine_);
}
```

### Custom Result Types

The simulator works with any type that supports:
- Default construction
- Copy/move semantics
- Arithmetic operations (+, -, *, /)
- Comparison operators (for min/max)

Example with custom type:

```cpp
struct MyResult {
    double value1;
    double value2;
    
    MyResult operator+(const MyResult& other) const {
        return {value1 + other.value1, value2 + other.value2};
    }
    // ... other operators
};

MonteCarloSimulator<MyResult> sim(10000);
```

## Performance Considerations

### Choosing Number of Simulations
- More simulations = better accuracy but slower
- Standard error decreases as 1/√N
- Typical ranges:
  - Quick estimates: 1,000 - 10,000
  - Moderate accuracy: 100,000 - 1,000,000
  - High accuracy: 10,000,000+

### Parallel vs Sequential
- Parallel is beneficial when:
  - Simulation function is computationally expensive
  - Number of simulations > 10,000
  - Multiple CPU cores available
- Sequential is better when:
  - Very simple simulation functions
  - Small number of simulations
  - Single-core environment

### Memory Usage
- Each simulation stores one result of type T
- For 1,000,000 simulations with double: ~8 MB
- Consider using `run()` instead of `runRaw()` if you only need statistics

## Common Use Cases

### Financial Modeling

```cpp
// Value at Risk (VaR) calculation
MonteCarloSimulator<double> sim(100000);
auto portfolio_value = sim.run([&]() {
    // Simulate portfolio returns
    return simulatePortfolio();
});

// 95% VaR is 5th percentile
```

### Physics Simulations

```cpp
// Particle scattering
MonteCarloSimulator<Vector3D> sim(1000000);
auto result = sim.run([&]() {
    return simulateParticleCollision();
});
```

### Game Theory

```cpp
// Strategy evaluation
MonteCarloSimulator<int> sim(10000);
auto wins = sim.run([&]() {
    return playGame(strategy1, strategy2);
});
double win_rate = static_cast<double>(wins.mean) / num_rounds;
```

## Testing

The library includes comprehensive tests in the `tests/` directory:

- `test_simulator.cpp`: Tests for the main simulator
- `test_random.cpp`: Tests for random number generation

Run tests:
```bash
cd build
ctest --output-on-failure
```

## Integration Examples

### CMake Integration

```cmake
# Method 1: As a subdirectory
add_subdirectory(path/to/MonteCarloSimulator)
target_link_libraries(your_target PRIVATE montecarlo)

# Method 2: After installation
find_package(MonteCarloSimulator REQUIRED)
target_link_libraries(your_target PRIVATE MonteCarloSimulator::montecarlo)
```

### Direct Include

```cpp
#include <montecarlo/montecarlo.h>

int main() {
    montecarlo::MonteCarloSimulator<double> sim(1000);
    // ... use the simulator
}
```

## Best Practices

1. **Use thread-local generators for parallel simulations**
   ```cpp
   auto& rng = montecarlo::getThreadLocalGenerator();
   ```

2. **Seed for reproducibility in tests**
   ```cpp
   RandomGenerator rng(fixed_seed);
   ```

3. **Choose appropriate data types**
   - Use `double` for continuous variables
   - Use `int` for discrete counts
   - Use custom types for multi-dimensional results

4. **Set confidence level if needed**
   ```cpp
   sim.setConfidenceLevel(0.99); // 99% CI
   ```

5. **Validate with analytical solutions when possible**
   - Compare Monte Carlo results with known values
   - Use convergence tests

## Troubleshooting

### Results are not converging
- Increase number of simulations
- Check simulation function for bugs
- Verify random number generation

### Slow performance
- Enable parallel execution
- Profile your simulation function
- Consider using simpler approximations

### Inconsistent results
- Set a fixed seed for reproducibility
- Check for race conditions in parallel execution
- Ensure thread-local generators are used

## Future Enhancements

Potential additions to the library:
- Variance reduction techniques (antithetic variates, control variates)
- Adaptive sampling
- GPU acceleration support
- More advanced statistical analysis
- Importance sampling
- Quasi-random sequences (Sobol, Halton)

## License

GPL-3.0 License - See LICENSE file
