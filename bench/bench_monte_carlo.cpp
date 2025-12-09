#include "montecarlo/montecarlo.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using montecarlo::make_engine;
#ifdef MCLIB_PARALLEL_ENABLED
using montecarlo::make_parallel_engine;
#endif
using montecarlo::make_sequential_engine;
using montecarlo::transform::Identity;
using montecarlo::WelfordAggregator;

namespace {
struct Options {
    std::uint64_t samples = 1'000'000;
    std::vector<std::size_t> threads{1, 2, 4};
    int repeats = 3;
    std::uint64_t seed = 123456789ULL;
};

double to_ms(std::chrono::steady_clock::duration d) {
    return std::chrono::duration<double, std::milli>(d).count();
}

// Parse comma-separated thread counts into a vector
std::vector<std::size_t> parse_thread_list(const std::string& arg) {
    std::vector<std::size_t> out;
    std::stringstream ss(arg);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) {
            out.push_back(static_cast<std::size_t>(std::stoul(token)));
        }
    }
    return out;
}

// Lightweight flag parsing for the benchmarks
Options parse_args(int argc, char** argv) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto require_value = [&](const char* name) {
            if (i + 1 >= argc) {
                throw std::runtime_error(std::string("missing value after ") + name);
            }
            return std::string(argv[++i]);
        };

        if (a == "--samples") {
            opts.samples = std::stoull(require_value("--samples"));
        } else if (a == "--threads") {
            opts.threads = parse_thread_list(require_value("--threads"));
        } else if (a == "--repeats") {
            opts.repeats = std::stoi(require_value("--repeats"));
        } else if (a == "--seed") {
            opts.seed = std::stoull(require_value("--seed"));
        } else if (a == "--help" || a == "-h") {
            std::cout << "Usage: ./montecarlo_bench [--samples N] [--threads t1,t2] "
                         "[--repeats R] [--seed S]\n";
            std::exit(0);
        }
    }
    if (opts.threads.empty()) {
        opts.threads.push_back(1);
    }
    return opts;
}

// Simple uniform [0,1) generator for the sample model
struct UniformModel {
    template <typename RNG>
    double operator()(RNG& rng) const {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    }
};

// One CSV row worth of benchmark data
struct BenchRow {
    std::string section;
    std::size_t threads;
    int run_idx;
    std::uint64_t samples;
    double elapsed_ms;
    double throughput;
    double estimate;
    double variance;
};

// Run the engine with the requested thread count and collect timing and stats
BenchRow run_engine(std::size_t threads, int run_idx, const Options& opts) {
    UniformModel model;
    std::uint64_t samples = opts.samples;
#ifdef MCLIB_PARALLEL_ENABLED
    if (threads > 1) {
        auto engine = make_parallel_engine(model, threads, opts.seed);
        auto r = engine.run(samples);
        double throughput = samples / (r.elapsed_ms / 1000.0);
        return {"engine", threads, run_idx, samples, r.elapsed_ms, throughput, r.estimate, r.variance};
    }
#else
    (void)threads;
    threads = 1;
#endif
    auto engine = make_sequential_engine(model, opts.seed);
    auto r = engine.run(samples);
    double throughput = samples / (r.elapsed_ms / 1000.0);
    return {"engine", 1, run_idx, samples, r.elapsed_ms, throughput, r.estimate, r.variance};
}

// Deterministic value generator for aggregator-only benchmarks
double synthetic_value(std::uint64_t i) {
    return static_cast<double>(i % 1024) / 1024.0;
}

// Baseline loop that manually accumulates mean and variance
BenchRow bench_raw_loop(const Options& opts) {
    auto start = std::chrono::steady_clock::now();
    double sum = 0.0;
    double sumsq = 0.0;
    // Plain loop to measure aggregation cost
    for (std::uint64_t i = 0; i < opts.samples; ++i) {
        double x = synthetic_value(i);
        sum += x;
        sumsq += x * x;
    }
    auto end = std::chrono::steady_clock::now();
    double mean = sum / static_cast<double>(opts.samples);
    double variance = 0.0;
    if (opts.samples > 1) {
        variance = (sumsq - sum * sum / static_cast<double>(opts.samples)) /
            static_cast<double>(opts.samples - 1);
    }
    double elapsed_ms = to_ms(end - start);
    double throughput = opts.samples / (elapsed_ms / 1000.0);
    return {"aggregator_raw", 0, 0, opts.samples, elapsed_ms, throughput, mean, variance};
}

// Compare against using the Welford aggregator helper
BenchRow bench_welford_loop(const Options& opts) {
    WelfordAggregator<> agg;
    auto start = std::chrono::steady_clock::now();
    for (std::uint64_t i = 0; i < opts.samples; ++i) {
        agg.add(synthetic_value(i));
    }
    auto end = std::chrono::steady_clock::now();
    double elapsed_ms = to_ms(end - start);
    double throughput = opts.samples / (elapsed_ms / 1000.0);
    return {"aggregator_welford", 0, 0, opts.samples, elapsed_ms, throughput, agg.result(), agg.variance()};
}

// RNG loop without engine abstractions to gauge overhead
BenchRow bench_manual_rng(const Options& opts) {
    auto rng = montecarlo::make_rng(opts.seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    WelfordAggregator<> agg;

    auto start = std::chrono::steady_clock::now();
    for (std::uint64_t i = 0; i < opts.samples; ++i) {
        agg.add(dist(rng));
    }
    auto end = std::chrono::steady_clock::now();

    double elapsed_ms = to_ms(end - start);
    double throughput = opts.samples / (elapsed_ms / 1000.0);
    return {"abstraction_manual_rng", 1, 0, opts.samples, elapsed_ms, throughput, agg.result(), agg.variance()};
}

// RNG loop that runs through the engine abstraction for comparison
BenchRow bench_engine_rng(const Options& opts) {
    UniformModel model;
    auto engine = make_sequential_engine(model, opts.seed);
    auto r = engine.run(opts.samples);
    double throughput = opts.samples / (r.elapsed_ms / 1000.0);
    return {"abstraction_engine_rng", 1, 0, opts.samples, r.elapsed_ms, throughput, r.estimate, r.variance};
}

// Emit one CSV-formatted line
void print_row(const BenchRow& row) {
    std::cout << row.section << ","
              << row.threads << ","
              << row.run_idx << ","
              << row.samples << ","
              << std::fixed << std::setprecision(4) << row.elapsed_ms << ","
              << std::fixed << std::setprecision(2) << row.throughput << ","
              << std::fixed << std::setprecision(6) << row.estimate << ","
              << std::fixed << std::setprecision(6) << row.variance
              << "\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        // Parse CLI flags and set up defaults
        Options opts = parse_args(argc, argv);

        // CSV header so results can be piped into a file or spreadsheet
        std::cout << "section,threads,run,samples,elapsed_ms,throughput,estimate,variance\n";

        for (std::size_t threads : opts.threads) {
            for (int run = 0; run < opts.repeats; ++run) {
                print_row(run_engine(threads, run, opts));
            }
        }

        print_row(bench_raw_loop(opts));
        print_row(bench_welford_loop(opts));

        // Abstraction overhead (manual RNG loop vs engine)
        print_row(bench_manual_rng(opts));
        print_row(bench_engine_rng(opts));
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
