# inotify file watcher

This example demonstrates Linux `inotify` for filesystem event monitoring in C.

It covers:

- Creating an inotify file descriptor with `inotify_init1`
- Registering a watch with `inotify_add_watch`
- Waiting for readiness with `poll`
- Reading and decoding variable-length `struct inotify_event` records
- Keeping the event logic testable behind a small wrapper API

## Kernel requirement

The running Linux kernel must be built with user-space inotify support:

```text
CONFIG_INOTIFY_USER=y
```

This is normally enabled on general-purpose Linux distributions. For embedded Linux or custom kernel builds, enable `Inotify support for userspace` before running this example.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

## Run

Watch a directory:

```sh
./build/linux/inotify/inotify-watch /tmp
```

In another shell, create, modify, move, or delete files in that directory.

## Test

```sh
ctest --test-dir build --output-on-failure
```

The tests use a temporary directory under `/tmp`, register an inotify watch, create a file, and assert that an `IN_CREATE` event is received.

## Files

- `include/inotify_demo/watcher.h`: public wrapper API
- `src/watcher.c`: inotify setup, dispatch, and mask formatting
- `src/main.c`: CLI watcher using `poll`
- `tests/test_watcher.c`: self-contained unit/integration tests
