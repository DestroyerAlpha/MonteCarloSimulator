find_package(GTest QUIET)

if(GTest_FOUND)
    add_executable(montecarlo_tests
        basic_constant_one.cpp
    )
    
    target_link_libraries(montecarlo_tests PRIVATE 
        montecarlo::montecarlo 
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(montecarlo_tests)
else()
    message(WARNING "Google Test not found. Tests will not be built.")
    message(STATUS "Install with: sudo apt-get install libgtest-dev (Linux)")
    message(STATUS "           or: brew install googletest (macOS)")
endif()