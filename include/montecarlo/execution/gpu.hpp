#pragma once

#ifdef MCLIB_GPU_ENABLED

#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <stdexcept>
#include <memory>

namespace montecarlo::execution {

// CUDA error checking macro
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw std::runtime_error(std::string("CUDA error: ") + \
                                   cudaGetErrorString(err)); \
        } \
    } while(0)

// Kernel for Pi estimation (example)
__global__ void pi_estimation_kernel(unsigned long long seed, 
                                      size_t iterations_per_thread,
                                      double* results) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    // Initialize random state
    curandState state;
    curand_init(seed, idx, 0, &state);

    unsigned long long inside = 0;

    for (size_t i = 0; i < iterations_per_thread; ++i) {
        double x = curand_uniform_double(&state);
        double y = curand_uniform_double(&state);

        if (x * x + y * y <= 1.0) {
            inside++;
        }
    }

    results[idx] = 4.0 * inside / iterations_per_thread;
}

// Generic kernel that works with callable objects
template<typename T>
__global__ void monte_carlo_kernel(T* model,
                                    unsigned long long seed,
                                    size_t iterations_per_thread,
                                    double* results) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    curandState state;
    curand_init(seed, idx, 0, &state);

    double sum = 0.0;

    for (size_t i = 0; i < iterations_per_thread; ++i) {
        // Generate uniform random number
        double u = curand_uniform_double(&state);

        // Call model (this is simplified - real implementation would need
        // the model to be GPU-compatible)
        sum += (*model)(u);
    }

    results[idx] = sum / iterations_per_thread;
}

// Reduction kernel to combine thread results
__global__ void reduce_results(double* input, double* output, size_t n) {
    extern __shared__ double sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

    sdata[tid] = (i < n) ? input[i] : 0.0;
    __syncthreads();

    // Reduction in shared memory
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s && (i + s) < n) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

class GPU {
 public:
    GPU(int device_id = 0, int blocks = 256, int threads_per_block = 256)
        : device_id_(device_id), blocks_(blocks), threads_per_block_(threads_per_block) {
        CUDA_CHECK(cudaSetDevice(device_id_));

        // Get device properties
        cudaDeviceProp prop;
        CUDA_CHECK(cudaGetDeviceProperties(&prop, device_id_));
        device_name_ = prop.name;
        max_threads_ = prop.maxThreadsPerBlock;

        // Adjust blocks if needed
        if (threads_per_block_ > max_threads_) {
            threads_per_block_ = max_threads_;
        }
    }

    ~GPU() {
        cudaDeviceReset();
    }

    // Specialized run for Pi estimation (demo)
    template<typename Aggregator>
    void run_pi_estimation(Aggregator& agg, size_t iterations, uint64_t seed = 42) {
        size_t total_threads = blocks_ * threads_per_block_;
        size_t iterations_per_thread = (iterations + total_threads - 1) / total_threads;

        // Allocate device memory
        double* d_results;
        CUDA_CHECK(cudaMalloc(&d_results, total_threads * sizeof(double)));

        // Launch kernel
        pi_estimation_kernel<<<blocks_, threads_per_block_>>>(
            seed, iterations_per_thread, d_results);
        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaDeviceSynchronize());

        // Copy results back
        std::vector<double> h_results(total_threads);
        CUDA_CHECK(cudaMemcpy(h_results.data(), d_results, 
                             total_threads * sizeof(double),
                             cudaMemcpyDeviceToHost));

        // Aggregate results
        agg.reset();
        for (double result : h_results) {
            agg.add(result);
        }

        CUDA_CHECK(cudaFree(d_results));
    }

    // Generic run method with device function
    template<typename Model, typename Aggregator>
    void run(Model&& model, Aggregator& agg, size_t iterations, uint64_t seed = 42) {
        // For now, this delegates to CPU for models that aren't GPU-native
        // In a full implementation, you'd use CUDA dynamic parallelism
        // or provide GPU-compiled model variants

        // This is a placeholder that shows the structure
        // Real implementation would require Model to be GPU-compatible
        throw std::runtime_error(
            "Generic GPU execution requires GPU-compatible models. "
            "Use run_pi_estimation() for the Pi estimation demo, "
            "or implement your model as a CUDA kernel.");
    }

    const std::string& device_name() const { return device_name_; }
    int blocks() const { return blocks_; }
    int threads_per_block() const { return threads_per_block_; }
    size_t total_threads() const { return blocks_ * threads_per_block_; }

 private:
    int device_id_;
    int blocks_;
    int threads_per_block_;
    int max_threads_;
    std::string device_name_;
};

} // namespace montecarlo::execution

#endif // MCLIB_GPU_ENABLED