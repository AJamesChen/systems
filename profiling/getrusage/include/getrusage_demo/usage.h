#ifndef GETRUSAGE_DEMO_USAGE_H
#define GETRUSAGE_DEMO_USAGE_H

#include <sys/resource.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct getrusage_demo_usage_delta {
    struct timeval user_cpu;
    struct timeval system_cpu;
    long max_rss_kb;
    long minor_faults;
    long major_faults;
    long voluntary_context_switches;
    long involuntary_context_switches;
} getrusage_demo_usage_delta;

int getrusage_demo_snapshot(int who, struct rusage *usage);
getrusage_demo_usage_delta getrusage_demo_diff(const struct rusage *before,
                                               const struct rusage *after);
double getrusage_demo_timeval_seconds(struct timeval value);

#ifdef __cplusplus
}
#endif

#endif
