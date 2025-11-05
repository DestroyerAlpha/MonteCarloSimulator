cmake_minimum_required(VERSION 3.14)
project(MonteCarloSimulator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)

# Library target (header-only)
add_library(montecarlo INTERFACE)
target_include_directories(montecarlo INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(montecarlo INTERFACE /W4)
else()
    target_compile_options(montecarlo INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS montecarlo
    EXPORT MonteCarloSimulatorTargets
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT MonteCarloSimulatorTargets
    FILE MonteCarloSimulatorTargets.cmake
    NAMESPACE MonteCarloSimulator::
    DESTINATION lib/cmake/MonteCarloSimulator
)

# Generate package configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MonteCarloSimulatorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MonteCarloSimulatorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MonteCarloSimulatorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/MonteCarloSimulator
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MonteCarloSimulatorConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MonteCarloSimulatorConfigVersion.cmake"
    DESTINATION lib/cmake/MonteCarloSimulator
)
