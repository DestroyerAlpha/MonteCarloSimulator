cmake_minimum_required(VERSION 3.14)

# Pi estimation example
add_executable(pi_estimation pi_estimation.cpp)
target_link_libraries(pi_estimation PRIVATE montecarlo)

# Option pricing example
add_executable(option_pricing option_pricing.cpp)
target_link_libraries(option_pricing PRIVATE montecarlo)

# Numerical integration example
add_executable(integration integration.cpp)
target_link_libraries(integration PRIVATE montecarlo)

# Dice rolling example
add_executable(dice_rolling dice_rolling.cpp)
target_link_libraries(dice_rolling PRIVATE montecarlo)
