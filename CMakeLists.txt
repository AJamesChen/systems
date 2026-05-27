cmake_minimum_required(VERSION 3.16)

project(systems
    VERSION 0.1.0
    DESCRIPTION "Linux systems programming examples in C/C++"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(SYSTEMS_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(SYSTEMS_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

function(systems_configure_target target)
    target_compile_definitions(${target} PRIVATE _GNU_SOURCE)

    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )

    if(SYSTEMS_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE -Werror)
    endif()

    if(SYSTEMS_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

enable_testing()

add_subdirectory(linux/inotify)
