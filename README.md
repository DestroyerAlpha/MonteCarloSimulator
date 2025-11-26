# MonteCarloSimulator

MonteCarloSimulator is a small C++ library and example collection for Monte Carlo simulations and numerical integration. It provides a lightweight core, RNG utilities, execution backends (sequential, parallel, GPU hooks), and several example programs and tests to get started.

**Repository layout**
- `include/` — public headers (e.g. `montecarlo.hpp`, `core/*`, `execution/*`).
- `examples/` — example programs demonstrating usage.
- `tests/` — unit / small integration tests.
- `CMakeLists.txt` — top-level CMake build configuration.
- `LICENSE` — project license.

**Key headers**
- `include/montecarlo/montecarlo.hpp` — main public entrypoints.
- `include/montecarlo/core/engine.hpp` — core engine abstractions.
- `include/montecarlo/core/rng.hpp` — RNG utilities and adapters.
- `include/montecarlo/core/result.hpp` — result types and helpers.
- `include/montecarlo/execution/sequential.hpp` — sequential execution backend.
- `include/montecarlo/execution/parallel.hpp` — parallel (threaded) backend.
- `include/montecarlo/execution/gpu.hpp` — GPU execution hooks (if available).

Quick Overview
--------------
The library is modern C++ and targets C++17 or newer. It is intended to be built with CMake and works with common compilers (g++, clang++).

Build (Quick Start)
-------------------
Prerequisites:
- C++ compiler with C++17 (or newer) support (e.g. `g++`).
- CMake 3.15+ (or the version required by the top-level `CMakeLists.txt`).

Standard out-of-source build:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j
```

After a successful build, example and test binaries are placed under `build/examples` and `build/tests` respectively (or available as CMake targets). For example, to run the example bundle:

```bash
./build/examples/all_examples
```

Run tests with CTest from the `build` directory:

```bash
cd build
ctest --output-on-failure
```

Using the library (simple compile)
---------------------------------
If you prefer to build a small program that uses the headers directly without CMake:

```bash
g++ -std=c++17 -Iinclude examples/main.cpp -o example
./example
```

Or reference the library target from your `CMakeLists.txt` (if `MCLib`/`MCLib::MCLib` is exported by the provided config):

```cmake
find_package(MCLib REQUIRED)
add_executable(myapp src/main.cpp)
target_link_libraries(myapp PRIVATE MCLib::MCLib)
```

API Notes
---------
- The public API lives under the `montecarlo` headers inside `include/`.
- Core concepts include engines, RNG adapters, and result containers. Execution backends are separated under `execution/` so you can swap sequential, parallel, or GPU-backed runs easily.

API Examples (minimal)
----------------------
The short examples below are illustrative — they demonstrate the intended usage pattern. Exact class/field names may vary; use them as a starting point for calling into the headers under `include/`.

1) Simple sequential estimate (numerical integration)

```cpp
#include <montecarlo/montecarlo.hpp> // public header
#include <iostream>

int main() {
    // Create a default engine (sequential backend assumed)
    montecarlo::Engine engine; // illustrative API

    // Create a RNG / sampler object (adapter provided by the library)
    montecarlo::Rng rng;

    // Run a simulation: 1'000'000 samples to estimate integral of f(x)=x^2 over [0,1]
    auto result = engine.run(1'000'000, [&](auto &sampler) {
        double x = sampler(rng); // sampler produces uniform sample in [0,1)
        return x * x;
    });

    std::cout << "Estimate: " << result.mean << " ± " << result.std_error << "\n";
    return 0;
}
```

2) Parallel execution (threaded)

```cpp
#include <montecarlo/montecarlo.hpp>
#include <montecarlo/execution/parallel.hpp> // enable parallel backend
#include <iostream>

int main() {
    // Construct a parallel engine (API shape is illustrative)
    montecarlo::execution::ParallelEngine engine(/*threads=*/std::thread::hardware_concurrency());

    auto result = engine.run(10'000'000, [](auto &sampler) {
        double x = sampler();
        return std::exp(-x * x);
    });

    std::cout << "Parallel estimate: " << result.mean << "\n";
}
```

Notes
- These examples are intentionally compact. Inspect the real headers in `include/montecarlo` and `include/montecarlo/execution` for the exact type and function names used by this project.
- If `MCLIB_ENABLE_PARALLEL` is enabled (CMake option), link and include the parallel backend; if `MCLIB_ENABLE_GPU` is enabled, GPU-specific headers and types may be available.

Examples
--------
- `examples/` contains small demos: numerical integration, option pricing, and pi estimation.
- Look at `examples/main.cpp` and the `examples/CMakeLists.txt` to see how examples are wired to the library.

Contributing
------------
- Contributions are welcome — please open issues or pull requests.
- Follow the existing code style and add tests for new functionality.

License
-------
This project includes a `LICENSE` file at the repository root. Refer to it for licensing terms.

Contact
-------
If you want help getting started or adding features, open an issue or contact the repository owner.

Enjoy experimenting with Monte Carlo methods!
# Modular Monte Carlo Simulation Framework (Design Using C++)

This repository contains the implementation of a modular Monte Carlo simulation framework in modern C++20 for **COMS W4995 – Design Using C++**.

The core idea is to provide a **generic `SimulationEngine`** that can run Monte Carlo simulations for different models (π estimation, option pricing, particle systems, etc.) while enforcing a simple, well-specified interface using C++20 **concepts**.

This README focuses on the components implemented by **Vince-Arvin Magno**:

* The `TrialModel` concept
* The single-threaded `SimulationEngine`
* The `Result` / confidence interval helpers
* The Monte Carlo π example
* A small constant-output test model

Other team members will extend this with multi-threaded backends, additional models, and cross-library benchmarks.

---

## Directory Structure

Expected layout:

```text
.
├── CMakeLists.txt
├── include/
│   └── mc/
│       └── core.hpp
├── examples/
│   └── pi.cpp
└── tests/
    └── basic_constant_one.cpp
```

* `include/mc/core.hpp` – Core generic Monte Carlo engine (`TrialModel`, `SimulationEngine`, `Result`, `ci_95`).
* `examples/pi.cpp` – Monte Carlo π estimation program using `SimulationEngine`.
* `tests/basic_constant_one.cpp` – Sanity test using a constant-valued model.

---

## Build Instructions

### Prerequisites

* A C++20-capable compiler:

  * **g++ (MinGW-w64 / MSYS2)** or
  * **MSVC (Visual Studio)** or
  * any modern Clang/GCC on Linux/macOS
* CMake (optional but recommended)

### Option 1: Build directly with g++ (Windows MSYS2 MinGW64)

From the project directory:

```bash
g++ -std=c++20 -O3 -Iinclude examples/pi.cpp -o example_pi.exe
g++ -std=c++20 -O3 -Iinclude tests/basic_constant_one.cpp -o test_constant_one.exe
```

Run:

```bash
./example_pi.exe 10000000
./test_constant_one.exe
```

### Option 2: Build with MSVC (Developer Command Prompt)

```bat
cd "C:\path\to\Design Using C++ Project"
cl /std:c++20 /O2 /Iinclude examples\pi.cpp /Fe:example_pi.exe
cl /std:c++20 /O2 /Iinclude tests\basic_constant_one.cpp /Fe:test_constant_one.exe

example_pi.exe
test_constant_one.exe
```

### Option 3: Build with CMake

Example `CMakeLists.txt` entries (only relevant parts):

```cmake
cmake_minimum_required(VERSION 3.16)
project(montecarlo_framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mc INTERFACE)
target_include_directories(mc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example_pi examples/pi.cpp)
target_link_libraries(example_pi PRIVATE mc)

add_executable(test_constant_one tests/basic_constant_one.cpp)
target_link_libraries(test_constant_one PRIVATE mc)
```

Then:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j

./example_pi          # or example_pi.exe on Windows
./test_constant_one
```

---

## Vince’s Component: Core SimulationEngine and π Example

### `TrialModel` Concept

A Monte Carlo model is any type `M` that satisfies the `TrialModel` concept, defined in `include/mc/core.hpp`:

```cpp
template <typename M, typename Rng>
concept TrialModel =
    requires(M m, Rng& rng) {
        { m.trial(rng) } -> std::convertible_to<double>;
    };
```

A valid model must define:

```cpp
double M::trial(Rng& rng);
```

which returns a `double` sample given a random number generator `rng`.

This allows the engine to be **generic over the model** (π estimation, option pricing, etc.) while enforcing a minimal interface.

---

### SimulationEngine

The core single-threaded engine is the class template:

```cpp
template <typename M, typename RngFactory, typename Estimator>
class SimulationEngine {
public:
    using CountType = std::uint64_t;

    SimulationEngine(M model,
                     RngFactory rng_factory,
                     Estimator transform,
                     std::uint64_t base_seed = 123456789ULL);

    [[nodiscard]] Result run_sequential(CountType n) const;
};
```

Template parameters:

* `M` – a model type satisfying `TrialModel`
* `RngFactory` – a callable that takes a `std::uint64_t` base seed and returns an RNG instance (e.g. `std::mt19937_64`)
* `Estimator` – a callable that transforms each raw sample into the final quantity of interest (e.g. indicator → estimate)

The `run_sequential(n)` method:

* Creates a single RNG using the `rng_factory`
* Runs `n` trials by repeatedly calling `model.trial(rng)`
* Applies the `Estimator` to each sample
* Computes:

  * the running mean,
  * unbiased sample variance,
  * standard error,
  * and wall-clock time in milliseconds

The numerical updates use **Welford’s algorithm**, which is stable and single-pass.

---

### Result and Confidence Interval

`Result` captures the outcome of a run:

```cpp
struct Result {
    double estimate{};
    double variance{};
    double standard_error{};
    std::uint64_t n{};
    double elapsed_ms{};
};
```

A helper function computes a 95% confidence interval:

```cpp
struct CI {
    double lo;
    double hi;
};

inline CI ci_95(const Result& r) {
    constexpr double z = 1.96;
    return { r.estimate - z * r.standard_error,
             r.estimate + z * r.standard_error };
}
```

These utilities will be reused by other group members when they add multi-threaded and comparative benchmarking backends.

---

## π Estimation Example (`examples/pi.cpp`)

The π model samples points in `[0,1] × [0,1]` and returns 1.0 if the point is inside the unit quarter circle:

```cpp
struct PiModel {
    template <typename Rng>
    double trial(Rng& rng) const {
        std::uniform_real_distribution<double> U(0.0, 1.0);
        double x = U(rng);
        double y = U(rng);
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    }
};
```

In `main`, this model is plugged into the engine:

```cpp
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
```

This uses the fact that:

* `P[(x, y) ∈ quarter circle] = π / 4`
* So `E[4 * indicator] = π`

The program prints the estimated π, standard error, a 95% CI, and runtime.

---

## Constant-One Test (`tests/basic_constant_one.cpp`)

To sanity-check the engine on a trivial model, we include a constant-output model:

```cpp
struct ConstantOneModel {
    template <typename Rng>
    double trial(Rng& rng) const {
        (void)rng;
        return 1.0;
    }
};
```

Using this with the engine and identity transform, the estimate should be very close to 1.0. The test prints the mean, variance, and standard error, and prints `TEST PASS` if the mean lies within a small tolerance of 1.0.

This helps confirm that the mean/variance logic inside `SimulationEngine::run_sequential` is correct.

---

## Single-Threaded Baseline Results (Vince)

Using `example_pi` with different sample sizes on a single thread produces:

| Samples (N) | Estimate | Standard Error | 95% CI             | Time (ms) |
| ----------: | -------: | -------------- | ------------------ | --------: |
|   1,000,000 |  3.13842 | 0.00164        | [3.13520, 3.14165] |     15.53 |
|  10,000,000 |  3.14155 | 0.00052        | [3.14053, 3.14256] |    159.96 |
|  50,000,000 |  3.14134 | 0.00023        | [3.14089, 3.14180] |    780.18 |

Observations:

* The estimate stays close to the true value of π ≈ 3.14159.
* The **standard error** decreases roughly on the order of `1 / sqrt(N)`.
* The width of the 95% confidence interval shrinks as `N` increases.
* Runtime scales roughly linearly with the number of samples, as expected for a single-threaded Monte Carlo algorithm.

These results both validate the engine and provide a baseline for the multi-threaded and cross-library comparisons to be implemented by other group members.

---

## Division of Work (Vince’s Contributions)

* Implemented the **`TrialModel` concept** and the generic **single-threaded `SimulationEngine`** in C++20.
* Implemented the **`Result`** type and **`ci_95`** helper function.
* Designed and coded the **Monte Carlo π example** (`PiModel` + `examples/pi.cpp`).
* Produced **single-threaded baseline measurements** for several sample sizes.
* Added a **constant-output test model** to validate the statistical correctness of the engine.
