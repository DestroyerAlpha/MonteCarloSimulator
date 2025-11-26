# MonteCarloSimulator

MonteCarloSimulator is a small, header-first C++ library for building Monte Carlo experiments and numerical estimators. It provides:

- A generic, concept-driven `SimulationEngine` that accepts a model, an execution policy, an aggregator, and an optional transform.
- Lightweight aggregators (Welford, histogram) and helpers for estimates and confidence intervals.
- Execution policies: `execution::Sequential` (default) and `execution::Parallel` (threaded). GPU hooks exist behind the `MCLIB_ENABLE_GPU` CMake option.

The public API is exposed via the umbrella header `include/montecarlo/montecarlo.hpp`.

Repository layout
---------------
- `include/` — public headers (namespace `montecarlo`).
- `examples/` — standalone example programs (e.g. π estimation, option pricing).
- `tests/` — small tests and sanity checks.
- `CMakeLists.txt` — top-level build script and options.
- `LICENSE` — project license.

Minimum requirements
--------------------
- CMake 3.18+ (project uses features requiring 3.18+).
- A C++20-capable compiler (project targets C++20 via CMake).

CMake options of interest
-------------------------
- `MCLIB_BUILD_EXAMPLES` (ON/off) — build `examples/`.
- `MCLIB_BUILD_TESTS` (ON/off) — build `tests/` and enable CTest.
- `MCLIB_ENABLE_PARALLEL` (ON/off) — enable/disable parallel execution support (requires Threads).
- `MCLIB_ENABLE_GPU` (OFF by default) — enable CUDA GPU hooks.

Quick build
-----------
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -- -j
```

Run examples (when `MCLIB_BUILD_EXAMPLES=ON`):

```bash
./build/examples/all_examples
```

Run tests:

```bash
cd build
ctest --output-on-failure
```

Public API overview
-------------------
Key headers (public):

- `#include <montecarlo/montecarlo.hpp>` — umbrella include.
- `include/montecarlo/core/engine.hpp` — `SimulationEngine` and factory helpers (`make_engine`, `make_sequential_engine`, `make_parallel_engine`).
- `include/montecarlo/core/result.hpp` — `Result`, `ConfidenceInterval`, `ci_95`, and aggregators (`WelfordAggregator`, `HistogramAggregator`).
- `include/montecarlo/core/rng.hpp` — `make_rng` and `DefaultRngFactory` for reproducible RNG seeding.
- `include/montecarlo/core/transform.hpp` — common transforms in `montecarlo::transform` (Identity, LinearScale, Exp, Indicator, ...).

Concepts
--------
The library uses C++20 concepts to describe valid models and components:

- `TrialModel<Model, RNG>` — type `M` must provide `double M::trial(RNG& rng)`.
- `CallableModel<Model, RNG>` — models may alternatively be callable via `double M::operator()(RNG& rng)`.
- `SimulationModel` — either `TrialModel` or `CallableModel`.
- `Transform` — callable that converts a `double` to `double`.
- `ResultAggregator` — aggregator types must support `add()`, `result()`, and `reset()`.
 - `RngFactory` — a callable that accepts a `std::uint64_t` seed and returns a Uniform Random Bit Generator (URBG) type (for example, `std::mt19937_64`). The engine and execution policies accept a factory so users can inject custom RNG implementations or test stubs.

Core types
----------
- `SimulationEngine<Model, Aggregator = WelfordAggregator<>, ExecutionPolicy = execution::Sequential, Transform = transform::Identity>` — main engine template. Use the provided factory helpers for convenience.
- `Result` — returned by `SimulationEngine::run(...)` with fields:
  - `estimate` (double)
  - `variance` (double)
  - `standard_error` (double)
  - `iterations` (uint64_t)
  - `elapsed_ms` (double)
- `ConfidenceInterval` and helper `ci_95(const Result&)`.

Factory helpers
---------------
Use the helpers to create commonly-configured engines:

- `make_sequential_engine(model, seed = 123456789ULL, transform = transform::Identity{})`
- `make_parallel_engine(model, threads = 0, seed = 123456789ULL, transform = transform::Identity{})`

Minimal example — π estimation (compile-ready)
---------------------------------------------
This example uses the actual public API and compiles when the include path is set.

```cpp
#include <montecarlo/montecarlo.hpp>
#include <random>
#include <iostream>

struct PiModel {
    template<typename Rng>
    double trial(Rng &rng) const {
        std::uniform_real_distribution<double> U(0.0, 1.0);
        double x = U(rng);
        double y = U(rng);
        return (x * x + y * y <= 1.0) ? 1.0 : 0.0;
    }
};

int main() {
    using namespace montecarlo;

    PiModel model;
    auto engine = make_sequential_engine(model); // uses execution::Sequential and Identity transform

    std::uint64_t N = 1'000'000;
    Result r = engine.run(N);

    auto ci = ci_95(r);
    std::cout << "pi estimate = " << r.estimate * 4.0 << " (raw mean=" << r.estimate << ")\n";
    std::cout << "stderr=" << r.standard_error * 4.0 << " 95% CI=[" << (ci.lower * 4.0) << ", " << (ci.upper * 4.0) << "]\n";
    std::cout << "time ms=" << r.elapsed_ms << "\n";
    return 0;
}
```

Notes: the example above assumes the model's `trial()` returns the indicator (1 inside quarter circle, 0 otherwise). We multiply by 4 when reporting π.

Parallel example
----------------
If `MCLIB_ENABLE_PARALLEL` is enabled, construct a parallel engine with the convenience factory:

```cpp
#include <montecarlo/montecarlo.hpp>
#include <iostream>

int main() {
    using namespace montecarlo;
    PiModel model;
    auto engine = make_parallel_engine(model, /*threads=*/std::thread::hardware_concurrency());
    auto r = engine.run(5'000'000);
    std::cout << "Parallel estimate (raw mean) = " << r.estimate << "\n";
}
```

Advanced topics
---------------
- Replace the default aggregator with `WelfordAggregator<>` (default) or `HistogramAggregator<>` to collect distributional information.
- Use `transform::LinearScale`, `transform::Indicator`, or compose transforms to adapt raw trial output into estimands.
- For reproducible parallel runs, `make_rng(seed, stream_id)` and the engine's `seed()` / `set_seed()` methods are available.

RNG factory and injection
-------------------------
The library exposes an `RngFactory` concept: a callable that takes a `std::uint64_t` and returns a type satisfying `std::uniform_random_bit_generator`. A default implementation `DefaultRngFactory` is provided (it returns an `std::mt19937_64` seeded with `make_rng`).

Why inject a factory?
- Lets users choose different RNG engines (PCG, xorshift, std engines, or cryptographic generators).
- Enables deterministic, reproducible tests by injecting a `StubFactory` that returns a trivial generator.
- Keeps execution policies generic while allowing them to create per-worker RNG instances.

How parallel seeding works
- Execution policies derive per-worker seeds from the base seed. The current policy implementation uses `rng_factory(base_seed + thread_id)` to give each worker a deterministic, distinct stream. This is simple and portable; if you need sophisticated streams, provide a factory that internally uses `std::seed_seq` or a two-argument `make_rng(seed, stream_id)` helper.

Example: stub RNG factory for tests

```cpp
// A tiny deterministic stub generator (meets URBG requirements)
struct StubRng {
    using result_type = uint64_t;
    result_type operator()() { return 42; }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
};

struct StubFactory {
    StubRng operator()(std::uint64_t) const noexcept { return StubRng{}; }
};

// Use in tests
PiModel model;
auto engine = make_sequential_engine(model, StubFactory{}, /*seed*/ 0);
auto r = engine.run(1000); // deterministic output from stub RNG
```

Example: custom RNG (PCG-like) factory

```cpp
struct MyPcgFactory {
    std::mt19937_64 operator()(std::uint64_t seed) const {
        // initialize your PCG or other engine here; using mt19937_64 as a placeholder
        return ::montecarlo::make_rng(seed);
    }
};

auto engine = make_parallel_engine(model, /*threads=*/4, MyPcgFactory{}, /*seed=*/42);
```

License
-------
See the `LICENSE` file at the repository root.
