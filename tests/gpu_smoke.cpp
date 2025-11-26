#include <gtest/gtest.h>
#include <cstdint>
#include <iostream>

// This test verifies that the GPU execution object can be constructed
#ifdef MCLIB_GPU_ENABLED
#include "montecarlo/montecarlo.hpp"

TEST(GPUSmoke, Construct) {
    // Construct with default device
    EXPECT_NO_THROW({
        montecarlo::execution::GPU gpu;
        std::cout << "Max Threads: " << gpu.max_threads() << std::endl
                  << "Device Name: " << gpu.device_name() << std::endl;
    });
}
#else

TEST(GPUSmoke, Disabled) {
    GTEST_SKIP() << "GPU support not enabled (MCLIB_GPU_ENABLED not set)";
}

#endif
