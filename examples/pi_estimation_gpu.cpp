#include <iostream>
#ifdef MCLIB_GPU_ENABLED

#include "montecarlo/montecarlo.hpp"
#include "montecarlo/execution/gpu.hpp"
#include <iomanip>
#include <chrono>

int main() {
    std::cout << "=== GPU-Accelerated Pi Estimation ===" << std::endl;
    std::cout << "True value of Pi: " << M_PI << std::endl << std::endl;
    
    try {
        mclib::execution::GPU gpu_policy;
        
        std::cout << "GPU Device: " << gpu_policy.device_name() << std::endl;
        std::cout << "Grid: " << gpu_policy.blocks() << " blocks × " 
                  << gpu_policy.threads_per_block() << " threads = "
                  << gpu_policy.total_threads() << " total threads" << std::endl << std::endl;
        
        std::vector<size_t> sample_sizes = {
            1000000,      // 1M
            10000000,     // 10M
            100000000,    // 100M
            1000000000    // 1B
        };
        
        std::cout << "GPU Execution:" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        std::cout << std::setw(15) << "Samples" 
                  << std::setw(15) << "Estimate" 
                  << std::setw(15) << "Error" 
                  << std::setw(15) << "Std Error"
                  << std::setw(10) << "Time (s)" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        for (size_t n : sample_sizes) {
            mclib::MeanAggregator<> agg;
            
            auto start = std::chrono::high_resolution_clock::now();
            gpu_policy.run_pi_estimation(agg, n);
            auto end = std::chrono::high_resolution_clock::now();
            
            std::chrono::duration<double> elapsed = end - start;
            double error = std::abs(agg.result() - M_PI);
            
            std::cout << std::setw(15) << n
                      << std::setw(15) << std::fixed << std::setprecision(6) << agg.result()
                      << std::setw(15) << std::scientific << std::setprecision(2) << error
                      << std::setw(15) << std::fixed << std::setprecision(6) << agg.std_error()
                      << std::setw(10) << std::fixed << std::setprecision(4) << elapsed.count()
                      << std::endl;
        }
        
        // Compare with CPU
        std::cout << "\n\nCPU Parallel Comparison:" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        class PiModel {
        public:
            template<typename RNG>
            double operator()(RNG& rng) {
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                double x = dist(rng);
                double y = dist(rng);
                return (x * x + y * y <= 1.0) ? 4.0 : 0.0;
            }
        };
        
        auto cpu_engine = mclib::make_parallel_engine(PiModel{});
        
        for (size_t n : {1000000, 10000000, 100000000}) {
            auto result = cpu_engine.simulate(n);
            double error = std::abs(result.value - M_PI);
            
            std::cout << std::setw(15) << n
                      << std::setw(15) << std::fixed << std::setprecision(6) << result.value
                      << std::setw(15) << std::scientific << std::setprecision(2) << error
                      << std::setw(15) << std::fixed << std::setprecision(6) << result.std_error
                      << std::setw(10) << std::fixed << std::setprecision(4) << result.elapsed_seconds
                      << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

#else

int main() {
    std::cout << "GPU support not enabled. Rebuild with -DMCLIB_ENABLE_GPU=ON" << std::endl;
    return 1;
}

#endif // MCLIB_GPU_ENABLED