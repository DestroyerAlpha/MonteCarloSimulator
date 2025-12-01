set(TEST_SOURCES
    unit_tests.cpp
)

add_executable(montecarlo_tests ${TEST_SOURCES})
target_link_libraries(montecarlo_tests PRIVATE montecarlo::montecarlo)
target_include_directories(montecarlo_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_test(NAME montecarlo_unit_tests COMMAND montecarlo_tests)
