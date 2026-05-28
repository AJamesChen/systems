#include "getrusage_demo/usage.h"

#include <stddef.h>
#include <sys/resource.h>
#include <sys/time.h>

static struct timeval timeval_subtract(struct timeval after, struct timeval before)
{
    struct timeval result = {
        .tv_sec = after.tv_sec - before.tv_sec,
        .tv_usec = after.tv_usec - before.tv_usec,
    };

    if (result.tv_usec < 0) {
        result.tv_sec -= 1;
        result.tv_usec += 1000000;
    }

    return result;
}

int getrusage_demo_snapshot(int who, struct rusage *usage)
{
    if (usage == NULL) {
        return -1;
    }

    return getrusage(who, usage);
}

getrusage_demo_usage_delta getrusage_demo_diff(const struct rusage *before,
                                               const struct rusage *after)
{
    getrusage_demo_usage_delta delta = {
        .user_cpu = {0},
        .system_cpu = {0},
        .max_rss_kb = 0,
        .minor_faults = 0,
        .major_faults = 0,
        .voluntary_context_switches = 0,
        .involuntary_context_switches = 0,
    };

    if (before == NULL || after == NULL) {
        return delta;
    }

    delta.user_cpu = timeval_subtract(after->ru_utime, before->ru_utime);
    delta.system_cpu = timeval_subtract(after->ru_stime, before->ru_stime);
    delta.max_rss_kb = after->ru_maxrss;
    delta.minor_faults = after->ru_minflt - before->ru_minflt;
    delta.major_faults = after->ru_majflt - before->ru_majflt;
    delta.voluntary_context_switches = after->ru_nvcsw - before->ru_nvcsw;
    delta.involuntary_context_switches = after->ru_nivcsw - before->ru_nivcsw;

    return delta;
}

double getrusage_demo_timeval_seconds(struct timeval value)
{
    return (double)value.tv_sec + ((double)value.tv_usec / 1000000.0);
}
