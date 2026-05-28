#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    const char *trace_file = getenv("MALLOC_TRACE");
    if (trace_file == NULL || trace_file[0] == '\0') {
        fprintf(stderr, "set MALLOC_TRACE to the trace output path before running\n");
        return EXIT_FAILURE;
    }

    printf("writing malloc trace to %s\n", trace_file);
    mtrace();

    for (int j = 0; j < 2; j++) {
        void *leak = malloc(100); /* Never freed: a memory leak. */
        (void)leak;
    }

    void *calloc_leak = calloc(16, 16); /* Never freed: a memory leak. */
    (void)calloc_leak;

    muntrace();

    exit(EXIT_SUCCESS);
}
