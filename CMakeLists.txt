cmake_minimum_required(VERSION 3.16)
project(montecarlo_framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mc INTERFACE)
target_include_directories(mc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example_pi examples/pi.cpp)
target_link_libraries(example_pi PRIVATE mc)

add_executable(test_constant_one tests/basic_constant_one.cpp)
target_link_libraries(test_constant_one PRIVATE mc)
