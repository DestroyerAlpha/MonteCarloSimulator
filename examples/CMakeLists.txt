add_executable(all_examples 
    main.cpp
    pi_estimation.cpp
    option_pricing.cpp
    numerical_integration.cpp
)
target_link_libraries(all_examples PRIVATE montecarlo::montecarlo)

if(MCLIB_ENABLE_GPU)
    add_executable(pi_estimation_gpu pi_estimation_gpu.cu)
    target_link_libraries(pi_estimation_gpu PRIVATE montecarlo::montecarlo)
    set_target_properties(pi_estimation_gpu PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CMAKE_CUDA_ARCHITECTURES native
    )
endif()