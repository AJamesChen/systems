# systems

Linux systems programming, networking, concurrency, embedded Linux, RTOS, unit testing, benchmarking, and CI/CD examples in C/C++.

## Examples

| Area | Example | Description |
| --- | --- | --- |
| Linux systems programming | [inotify file watcher](linux/inotify) | Watches filesystem changes with `inotify`, `poll`, unit tests, CMake, and GitHub Actions CI. Requires kernel `CONFIG_INOTIFY_USER`. |
| Profiling | [getrusage profiler](profiling/getrusage) | Measures user CPU, system CPU, max RSS, page faults, and context switches with `getrusage`. |

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

## CI

GitHub Actions builds the examples with sanitizers enabled and runs the test suite on Ubuntu.
