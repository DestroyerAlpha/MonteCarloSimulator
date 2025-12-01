add_executable(montecarlo_bench
    bench_monte_carlo.cpp
)

target_link_libraries(montecarlo_bench PRIVATE montecarlo::montecarlo)
