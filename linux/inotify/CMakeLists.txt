add_library(inotify_demo STATIC
    src/watcher.c
)

target_include_directories(inotify_demo
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

systems_configure_target(inotify_demo)

add_executable(inotify-watch
    src/main.c
)

target_link_libraries(inotify-watch PRIVATE inotify_demo)
systems_configure_target(inotify-watch)

add_executable(inotify_demo_tests
    tests/test_watcher.c
)

target_link_libraries(inotify_demo_tests PRIVATE inotify_demo)
systems_configure_target(inotify_demo_tests)

add_test(NAME inotify_demo_tests COMMAND inotify_demo_tests)
set_tests_properties(inotify_demo_tests PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
)
