cmake_minimum_required(VERSION 3.14)

# Simple test framework - no external dependencies
add_executable(test_simulator test_simulator.cpp)
target_link_libraries(test_simulator PRIVATE montecarlo)

add_executable(test_random test_random.cpp)
target_link_libraries(test_random PRIVATE montecarlo)

# Add tests
add_test(NAME SimulatorTest COMMAND test_simulator)
add_test(NAME RandomTest COMMAND test_random)
