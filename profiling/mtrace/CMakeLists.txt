add_executable(mtrace-sample
    src/main.c
)

systems_configure_target(mtrace-sample NO_SANITIZERS)

target_compile_options(mtrace-sample PRIVATE -g -fno-pie)
target_link_options(mtrace-sample PRIVATE -no-pie)
