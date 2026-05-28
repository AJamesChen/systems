#include "getrusage_demo/usage.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#define MEMORY_BYTES (8UL * 1024UL * 1024UL)

static void print_usage(const char *program)
{
    fprintf(stderr, "Usage: %s [iterations]\n", program);
}

static int parse_iterations(const char *text, unsigned long *iterations)
{
    if (text == NULL || iterations == NULL) {
        return -1;
    }

    errno = 0;
    char *end = NULL;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value == 0UL) {
        return -1;
    }

    *iterations = value;
    return 0;
}

static unsigned long run_cpu_work(unsigned long iterations)
{
    volatile unsigned long accumulator = 0UL;

    for (unsigned long i = 0UL; i < iterations; ++i) {
        accumulator += (i ^ (i >> 3U)) % 97UL;
    }

    return accumulator;
}

static int run_memory_work(size_t bytes, unsigned long *checksum)
{
    if (checksum == NULL) {
        return -1;
    }

    unsigned char *buffer = calloc(bytes, sizeof(*buffer));
    if (buffer == NULL) {
        return -1;
    }

    unsigned long accumulator = 0UL;
    for (size_t offset = 0U; offset < bytes; offset += 4096U) {
        buffer[offset] = (unsigned char)(offset / 4096U);
        accumulator += buffer[offset];
    }

    *checksum = accumulator;
    free(buffer);
    return 0;
}

static void print_delta(const getrusage_demo_usage_delta *delta)
{
    printf("user_cpu_seconds=%.6f\n", getrusage_demo_timeval_seconds(delta->user_cpu));
    printf("system_cpu_seconds=%.6f\n", getrusage_demo_timeval_seconds(delta->system_cpu));
    printf("max_rss_kb=%ld\n", delta->max_rss_kb);
    printf("minor_page_faults=%ld\n", delta->minor_faults);
    printf("major_page_faults=%ld\n", delta->major_faults);
    printf("voluntary_context_switches=%ld\n", delta->voluntary_context_switches);
    printf("involuntary_context_switches=%ld\n", delta->involuntary_context_switches);
}

int main(int argc, char **argv)
{
    unsigned long iterations = 50000000UL;
    if (argc > 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2 && parse_iterations(argv[1], &iterations) != 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    struct rusage before;
    struct rusage after;

    if (getrusage_demo_snapshot(RUSAGE_SELF, &before) != 0) {
        perror("getrusage before");
        return EXIT_FAILURE;
    }

    const unsigned long result = run_cpu_work(iterations);
    unsigned long memory_checksum = 0UL;
    if (run_memory_work(MEMORY_BYTES, &memory_checksum) != 0) {
        perror("memory workload");
        return EXIT_FAILURE;
    }

    if (getrusage_demo_snapshot(RUSAGE_SELF, &after) != 0) {
        perror("getrusage after");
        return EXIT_FAILURE;
    }

    const getrusage_demo_usage_delta delta = getrusage_demo_diff(&before, &after);

    printf("iterations=%lu\n", iterations);
    printf("work_result=%lu\n", result);
    printf("memory_bytes_touched=%lu\n", MEMORY_BYTES);
    printf("memory_checksum=%lu\n", memory_checksum);
    print_delta(&delta);

    return EXIT_SUCCESS;
}
