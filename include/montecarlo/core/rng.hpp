#pragma once
#include <random>
#include <cstdint>

namespace montecarlo {

// Better parallel seeding using seed sequences
inline std::mt19937_64 make_rng(std::uint64_t seed,
                                std::uint64_t stream_id = 0) {
    std::seed_seq seq{
        static_cast<unsigned>(seed),
        static_cast<unsigned>(seed >> 32),
        static_cast<unsigned>(stream_id),
        static_cast<unsigned>(stream_id >> 32)
    };
    return std::mt19937_64(seq);
}

// Default RNG factory
struct DefaultRngFactory {
    std::mt19937_64 operator()(std::uint64_t seed) const {
        return make_rng(seed);
    }
};

} // namespace montecarlo
