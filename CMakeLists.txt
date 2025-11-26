cmake_minimum_required(VERSION 3.18)
project(MonteCarloLib VERSION 1.0.0 LANGUAGES CXX)
#
# --- Global settings -------------------------------------------------------
#
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#
# --- Options ---------------------------------------------------------------
#
option(MCLIB_BUILD_EXAMPLES "Build example applications" ON)
option(MCLIB_BUILD_TESTS "Build unit tests" ON)
option(MCLIB_ENABLE_PARALLEL "Enable parallel CPU execution" ON)
option(MCLIB_ENABLE_GPU "Enable GPU acceleration (CUDA)" OFF)

# Compiler warnings
add_library(montecarlo_warnings INTERFACE)
target_compile_options(montecarlo_warnings INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -Werror>
)
# Find dependencies based on options
if(MCLIB_ENABLE_PARALLEL)
    find_package(Threads REQUIRED)
endif()

if(MCLIB_ENABLE_GPU)
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    target_compile_options(montecarlo_warnings INTERFACE
        $<$<COMPILE_LANGUAGE:CUDA>:>
    )
endif()

# Library target (header-only)
add_library(montecarlo INTERFACE)
add_library(montecarlo::montecarlo ALIAS montecarlo)


target_include_directories(montecarlo INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(montecarlo INTERFACE cxx_std_20)

# Configure compile definitions based on options
if(MCLIB_ENABLE_PARALLEL)
    target_compile_definitions(montecarlo INTERFACE MCLIB_PARALLEL_ENABLED)
    target_link_libraries(montecarlo INTERFACE Threads::Threads)
endif()

if(MCLIB_ENABLE_GPU)
    target_compile_definitions(montecarlo INTERFACE MCLIB_GPU_ENABLED)
    target_link_libraries(montecarlo INTERFACE CUDA::cudart)
endif()

target_link_libraries(montecarlo INTERFACE
    $<BUILD_INTERFACE:montecarlo_warnings>
)
# Examples
if(MCLIB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(MCLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS montecarlo
    EXPORT MCLibTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT MCLibTargets
    FILE MCLibTargets.cmake
    NAMESPACE montecarlo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MCLib
)

# Generate config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MClibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MClibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MCLib
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MClibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MClibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MClibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MCLib
)