#pragma once
#include <cstdint>

struct StubRng {
    using result_type = uint64_t;
    result_type operator()() { return 42; }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return UINT64_MAX; }
};

struct StubFactory {
    StubRng operator()(std::uint64_t) const noexcept { return StubRng{}; }
};
