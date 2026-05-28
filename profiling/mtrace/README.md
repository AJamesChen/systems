# mtrace malloc tracing

This example demonstrates glibc `mtrace(3)` malloc tracing with `<mcheck.h>`.

It intentionally leaks:

- two `malloc(100)` allocations
- one `calloc(16, 16)` allocation

`mtrace()` records allocation events to the file named by the `MALLOC_TRACE` environment variable. The generated trace is then decoded with the `mtrace` command.

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

## Run

Set `MALLOC_TRACE` to a writable output path, then run the sample:

```sh
LD_PRELOAD=libc_malloc_debug.so.0 \
MALLOC_TRACE=/tmp/mtrace-sample.log \
./build/profiling/mtrace/mtrace-sample
```

Decode the trace:

```sh
mtrace ./build/profiling/mtrace/mtrace-sample /tmp/mtrace-sample.log
```

Expected output includes the three leaked allocations from `src/main.c`:

```text
Memory not freed:
-----------------
           Address     Size     Caller
...
```

## Notes

`mtrace` is a glibc-specific debugging tool. It is useful for small demonstrations and quick allocation tracing, but AddressSanitizer, LeakSanitizer, or Valgrind are usually better choices for production leak detection workflows.

Modern glibc systems may require `LD_PRELOAD=libc_malloc_debug.so.0` for `mtrace()` to generate the trace file. Without it, the sample can run successfully but no `MALLOC_TRACE` output file is created.

This sample target is built without sanitizers even when `SYSTEMS_ENABLE_SANITIZERS=ON`, because LeakSanitizer intentionally reports the sample leaks before the `mtrace` workflow can be inspected.

If `mtrace` is not installed, install the package that provides glibc debugging utilities for your distribution. On Ubuntu, this is commonly available from `libc-dev-bin`.

## Files

- `src/main.c`: enables `mtrace()`, intentionally leaks memory, closes tracing with `muntrace()`, and exits successfully
