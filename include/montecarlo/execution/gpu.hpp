#pragma once

#ifdef MCLIB_GPU_ENABLED
#include <cuda_runtime.h>
#include <stdexcept>

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw std::runtime_error(std::string("CUDA error: ") + \
                                   cudaGetErrorString(err)); \
        } \
    } while(0)

namespace montecarlo::execution {
class GPU {
 public:
    GPU(int device_id = 0) : device_id_(device_id) {
        CUDA_CHECK(cudaSetDevice(device_id_));
        // Get device properties
        cudaDeviceProp prop;
        CUDA_CHECK(cudaGetDeviceProperties(&prop, device_id_));
        device_name_ = prop.name;
        max_threads_ = prop.maxThreadsPerBlock;
    }

    ~GPU() {
        cudaDeviceReset();
    }

    template<typename Model, typename Aggregator, typename RngFactory = ::montecarlo::DefaultRngFactory>
    void run(Model, Aggregator&, size_t, uint64_t, RngFactory) {
        throw std::runtime_error("GPU execution not implemented yet.");
    }

    const std::string& device_name() const { return device_name_; }
    int max_threads() { return max_threads_; }

 private:
    int device_id_;
    int max_threads_;
    std::string device_name_;
};

} // namespace montecarlo::execution

#endif // MCLIB_GPU_ENABLED