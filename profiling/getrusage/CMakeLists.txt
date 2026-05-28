add_library(getrusage_demo STATIC
    src/usage.c
)

target_include_directories(getrusage_demo
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

systems_configure_target(getrusage_demo)

add_executable(getrusage-sample
    src/main.c
)

target_link_libraries(getrusage-sample PRIVATE getrusage_demo)
systems_configure_target(getrusage-sample)

add_executable(getrusage_demo_tests
    tests/test_usage.c
)

target_link_libraries(getrusage_demo_tests PRIVATE getrusage_demo)
systems_configure_target(getrusage_demo_tests)

add_test(NAME getrusage_demo_tests COMMAND getrusage_demo_tests)
set_tests_properties(getrusage_demo_tests PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
)
