# MonteCarloSimulator

[![Build Status](https://github.com/DestroyerAlpha/MonteCarloSimulator/workflows/CI/badge.svg)](https://github.com/DestroyerAlpha/MonteCarloSimulator/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

A modern, header-only C++ library for building high-performance Monte Carlo simulations and numerical estimators.

## Features

✨ **Modern C++20 Design**
- Concept-driven API with compile-time type safety
- Zero-cost abstractions through template metaprogramming
- Clear, descriptive error messages

🚀 **Flexible Execution Policies**
- Sequential execution for small problems and debugging
- Parallel execution with automatic thread management
- GPU acceleration hooks (CUDA support planned)

📊 **Robust Statistical Aggregation**
- Welford's algorithm for numerically stable variance computation
- Histogram aggregation for distribution analysis
- Extensible aggregator interface

🔧 **Composable Architecture**
- Mix and match models, execution policies, aggregators, and transforms
- Easy to extend with custom components
- Plugin your own RNG implementations

📦 **Header-Only & Easy Integration**
- No linking required - just include headers
- Minimal dependencies (C++20 standard library)
- CMake integration support

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/DestroyerAlpha/MonteCarloSimulator.git
cd MonteCarloSimulator

# Build examples and tests
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j

# Run examples
./examples/all_examples

# Run tests
ctest --output-on-failure
```

### Your First Simulation

Estimate π using Monte Carlo integration:

```cpp
#include <montecarlo/montecarlo.hpp>
#include <iostream>

struct PiModel {
    template<typename Rng>
    double trial(Rng& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng), y = dist(rng);
        return (x*x + y*y <= 1.0) ? 4.0 : 0.0;  // Inside unit circle
    }
};

int main() {
    auto engine = montecarlo::make_sequential_engine(PiModel{});
    auto result = engine.run(1'000'000);
    
    std::cout << "π ≈ " << result.estimate 
              << " ± " << result.standard_error << "\n";
    std::cout << "Time: " << result.elapsed_ms << " ms\n";
}
```

### Parallel Execution

```cpp
// Use all available CPU cores
auto engine = montecarlo::make_parallel_engine(PiModel{});
auto result = engine.run(10'000'000);
```

## Documentation

- **[Design Documentation](DESIGN.md)** - Detailed architecture and design decisions
- **[Examples](examples/)** - Complete working examples including:
  - π estimation using circle method
  - Option pricing (Black-Scholes)
  - Numerical integration
  - Dice roll expectation estimation
- **API Reference** - See inline documentation in headers

The public API is exposed via the umbrella header `include/montecarlo/montecarlo.hpp`.

## Repository Structure

```
MonteCarloSimulator/
├── include/montecarlo/     # Public API headers
│   ├── core/              # Core engine, concepts, aggregators
│   └── execution/         # Execution policies (sequential, parallel, GPU)
├── examples/              # Example applications
├── tests/                 # Unit tests and sanity checks
├── bench/                 # Performance benchmarks
├── CMakeLists.txt         # Build configuration
├── README.md              # This file
├── DESIGN.md              # Architecture documentation
└── LICENSE                # GPL v3
```

## Requirements

- **CMake** 3.18 or later
- **C++20-capable compiler**:
  - GCC 10+
  - Clang 12+
  - MSVC 2019 16.8+
- **Optional**: CUDA Toolkit (for GPU support)

## Build Configuration

Customize the build with CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `MCLIB_BUILD_EXAMPLES` | ON | Build example programs |
| `MCLIB_BUILD_TESTS` | ON | Build tests and enable CTest |
| `MCLIB_BUILD_BENCHMARKS` | ON | Build performance benchmarks |
| `MCLIB_ENABLE_PARALLEL` | ON | Enable multi-threaded execution |
| `MCLIB_ENABLE_GPU` | OFF | Enable CUDA GPU acceleration |

**Example:**
```bash
cmake -DMCLIB_ENABLE_PARALLEL=ON -DMCLIB_BUILD_EXAMPLES=ON ..
```

## API Overview

### Core Headers

| Header | Components | Description |
|--------|------------|-------------|
| `montecarlo/montecarlo.hpp` | All-in-one | Umbrella header including all components |
| `core/engine.hpp` | `SimulationEngine`, factories | Main engine and convenience helpers |
| `core/result.hpp` | `Result`, `ConfidenceInterval` | Statistical results and aggregators |
| `core/rng.hpp` | `make_rng`, `DefaultRngFactory` | Random number generation |
| `core/transform.hpp` | Transforms | Data transformation functions |
| `core/concepts.hpp` | Concepts | Type constraints |

### C++20 Concepts

The library uses concepts for compile-time type safety:

| Concept | Requirements | Purpose |
|---------|--------------|---------|
| `SimulationModel<M, RNG>` | `M::trial(RNG&)` or `M::operator()(RNG&)` | Defines trial logic |
| `ResultAggregator<A>` | `add()`, `result()`, `reset()` | Collects trial results |
| `Transform<T>` | `operator()(double) -> double` | Post-processes values |
| `RngFactory<F>` | `operator()(uint64_t) -> URBG` | Creates RNG instances |

### Core Types

**`SimulationEngine<Model, Aggregator, ExecutionPolicy, Transform>`**

Main simulation coordinator (all template parameters have sensible defaults).

**`Result`** - Simulation output containing:
- `estimate` - Mean value
- `variance` - Sample variance
- `standard_error` - Standard error of the mean
- `iterations` - Number of trials executed
- `elapsed_ms` - Execution time in milliseconds

**`ConfidenceInterval`** - Statistical interval with helpers like `ci_95(result)`

### Factory Functions

Convenience helpers for common configurations:

```cpp
// Sequential execution (single-threaded)
auto engine = make_sequential_engine(model, seed);

// Parallel execution (multi-threaded)
auto engine = make_parallel_engine(model, num_threads, seed);

// Full customization
auto engine = make_engine<Model, Policy, Aggregator, Transform>(
    model, policy, seed, rng_factory, transform
);
```

## Usage Examples

### Basic Example: Estimating π

```cpp
#include <montecarlo/montecarlo.hpp>
#include <iostream>

struct PiModel {
    template<typename Rng>
    double trial(Rng& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double x = dist(rng);
        double y = dist(rng);
        // Return 1 if point is inside unit circle, 0 otherwise
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    }
};

int main() {
    using namespace montecarlo;
    
    auto engine = make_sequential_engine(PiModel{});
    auto result = engine.run(1'000'000);
    
    // Multiply by 4 to get π (quarter circle → full circle)
    double pi_estimate = result.estimate * 4.0;
    auto ci = ci_95(result);
    
    std::cout << "π estimate: " << pi_estimate << "\n";
    std::cout << "95% CI: [" << ci.lower * 4.0 << ", " 
              << ci.upper * 4.0 << "]\n";
    std::cout << "Std error: " << result.standard_error * 4.0 << "\n";
    std::cout << "Time: " << result.elapsed_ms << " ms\n";
}
```

### Parallel Execution

Speed up computation using multiple threads:

```cpp
#include <montecarlo/montecarlo.hpp>
#include <thread>

int main() {
    using namespace montecarlo;
    
    // Use all available CPU cores
    auto engine = make_parallel_engine(
        PiModel{},
        std::thread::hardware_concurrency()
    );
    
    auto result = engine.run(10'000'000);
    std::cout << "Parallel π estimate: " << result.estimate * 4.0 << "\n";
}
```

## Advanced Usage

### Custom Aggregators

Use `HistogramAggregator` to analyze distributions:

```cpp
using namespace montecarlo;

HistogramAggregator<> hist(100, 0.0, 1.0);  // 100 bins, range [0,1]
auto engine = SimulationEngine<PiModel, HistogramAggregator<>>(
    PiModel{}, execution::Sequential{}, transform::Identity{}, 
    DefaultRngFactory{}, 42
);
auto result = engine.run(10000);
// Access histogram bins: hist.histogram()
```

### Transforms

Apply transformations to trial results:

```cpp
using namespace montecarlo;

// Linear scaling: y = 2x + 1
auto engine = make_sequential_engine(
    model, 42, transform::LinearScale{2.0, 1.0}
);

// Indicator function: estimate P(X > threshold)
auto engine2 = make_sequential_engine(
    model, 42, transform::Indicator{0.5, true}
);
```

### Reproducibility

Control random number generation for deterministic results:

```cpp
// Set seed explicitly
auto engine = make_sequential_engine(model, /*seed=*/12345);

// Change seed dynamically
engine.set_seed(67890);

// Or use simulate() with custom seed
auto result = engine.simulate(1000, /*seed=*/99999);
```

## Custom RNG Factories

The library supports custom random number generators through the `RngFactory` concept.

### Why Custom RNG Factories?

- **Flexibility**: Use PCG, xorshift, or cryptographic generators
- **Testing**: Inject deterministic stubs for unit tests
- **Performance**: Optimize for specific use cases
- **Parallel Independence**: Each thread gets its own RNG stream

### Example: Stub RNG for Testing

```cpp
// Deterministic generator for unit tests
struct StubRng {
    using result_type = uint64_t;
    result_type operator()() { return 42; }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
};

struct StubFactory {
    StubRng operator()(std::uint64_t) const noexcept { 
        return StubRng{}; 
    }
};

// Use in tests for reproducible results
auto engine = make_sequential_engine(PiModel{}, StubFactory{}, 0);
auto result = engine.run(1000);  // Always produces same output
```

### Example: Custom RNG Engine

```cpp
struct CustomRngFactory {
    std::mt19937_64 operator()(std::uint64_t seed) const {
        // Use your preferred RNG (PCG, xorshift, etc.)
        return montecarlo::make_rng(seed);
    }
};

auto engine = make_parallel_engine(
    model, /*threads=*/4, CustomRngFactory{}, /*seed=*/42
);
```

### Parallel RNG Seeding

Each thread receives a unique, deterministic seed:
- Thread 0: `base_seed + 0`
- Thread 1: `base_seed + 1`
- Thread N: `base_seed + N`

This ensures independent, reproducible random streams across threads.
