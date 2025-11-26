add_executable(all_examples 
    main.cpp
    pi_estimation.cpp
    option_pricing.cpp
    numerical_integration.cpp
)
target_link_libraries(all_examples PRIVATE montecarlo::montecarlo)