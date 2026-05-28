#include "getrusage_demo/usage.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

static void require_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void test_timeval_seconds(void)
{
    const struct timeval value = {
        .tv_sec = 2,
        .tv_usec = 250000,
    };

    const double seconds = getrusage_demo_timeval_seconds(value);
    require_true(seconds > 2.249 && seconds < 2.251, "timeval converts to seconds");
}

static void test_diff_subtracts_counters(void)
{
    struct rusage before = {0};
    struct rusage after = {0};

    before.ru_utime.tv_sec = 1;
    before.ru_utime.tv_usec = 900000;
    after.ru_utime.tv_sec = 3;
    after.ru_utime.tv_usec = 100000;

    before.ru_stime.tv_sec = 4;
    before.ru_stime.tv_usec = 100000;
    after.ru_stime.tv_sec = 4;
    after.ru_stime.tv_usec = 400000;

    before.ru_minflt = 10;
    after.ru_minflt = 15;
    before.ru_majflt = 1;
    after.ru_majflt = 3;
    after.ru_maxrss = 4096;
    before.ru_nvcsw = 7;
    after.ru_nvcsw = 11;
    before.ru_nivcsw = 2;
    after.ru_nivcsw = 8;

    const getrusage_demo_usage_delta delta = getrusage_demo_diff(&before, &after);

    require_true(delta.user_cpu.tv_sec == 1 && delta.user_cpu.tv_usec == 200000,
                 "user CPU delta normalizes microseconds");
    require_true(delta.system_cpu.tv_sec == 0 && delta.system_cpu.tv_usec == 300000,
                 "system CPU delta is computed");
    require_true(delta.minor_faults == 5, "minor fault delta is computed");
    require_true(delta.major_faults == 2, "major fault delta is computed");
    require_true(delta.max_rss_kb == 4096, "max RSS reports current high-water mark");
    require_true(delta.voluntary_context_switches == 4, "voluntary switch delta is computed");
    require_true(delta.involuntary_context_switches == 6, "involuntary switch delta is computed");
}

static void test_snapshot_success(void)
{
    struct rusage usage;
    require_true(getrusage_demo_snapshot(RUSAGE_SELF, &usage) == 0, "snapshot succeeds");
}

int main(void)
{
    test_timeval_seconds();
    test_diff_subtracts_counters();
    test_snapshot_success();

    puts("PASS: getrusage demo tests");
    return EXIT_SUCCESS;
}
