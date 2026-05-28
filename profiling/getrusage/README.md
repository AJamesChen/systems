# getrusage profiler

This example demonstrates `getrusage(2)` for lightweight process resource profiling in C.

It covers:

- Capturing `RUSAGE_SELF` snapshots before and after a workload
- Computing user CPU time and system CPU time deltas
- Reporting memory high-water mark, page faults, and context switches
- Keeping resource accounting logic testable behind a small wrapper API

## Build

From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

## Run

Run the default CPU and memory workload:

```sh
./build/profiling/getrusage/getrusage-sample
```

Pass a custom iteration count:

```sh
./build/profiling/getrusage/getrusage-sample 100000000
```

Example output:

```text
iterations=50000000
work_result=2399999748
memory_bytes_touched=8388608
memory_checksum=261120
user_cpu_seconds=0.138418
system_cpu_seconds=0.002119
max_rss_kb=10012
minor_page_faults=2055
major_page_faults=0
voluntary_context_switches=0
involuntary_context_switches=1
```

## Notes

On Linux, `ru_maxrss` is reported in kilobytes and is a high-water mark, not a delta. Many other `struct rusage` fields are platform-specific or not maintained, so this example focuses on commonly useful Linux fields.

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Files

- `include/getrusage_demo/usage.h`: public resource snapshot and diff API
- `src/usage.c`: `getrusage` wrapper and delta helpers
- `src/main.c`: CLI workload profiler
- `tests/test_usage.c`: self-contained unit tests
