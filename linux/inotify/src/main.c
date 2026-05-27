#include "inotify_demo/watcher.h"

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t g_should_stop = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    /* Signal handlers can safely update sig_atomic_t state. */
    g_should_stop = 1;
}

static int print_event(const struct inotify_event *event, void *context)
{
    (void)context;

    char mask[256];
    (void)inotify_demo_format_mask(event->mask, mask, sizeof(mask));

    printf("wd=%d mask=%s", event->wd, mask);
    if (event->len > 0U && event->name[0] != '\0') {
        printf(" name=%s", event->name);
    }
    putchar('\n');
    fflush(stdout);

    return 0;
}

static void print_usage(const char *program)
{
    fprintf(stderr, "Usage: %s <path>\n", program);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, handle_signal) == SIG_ERR || signal(SIGTERM, handle_signal) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    inotify_demo_watcher watcher = {.fd = -1};
    if (inotify_demo_watcher_init(&watcher, true) != 0) {
        perror("inotify_init1");
        return EXIT_FAILURE;
    }

    /* Watch common directory-entry and file-content changes for this demo. */
    const uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO |
                          IN_ATTRIB | IN_CLOSE_WRITE | IN_DELETE_SELF | IN_MOVE_SELF;
    if (inotify_demo_watcher_add(&watcher, argv[1], mask) < 0) {
        perror("inotify_add_watch");
        inotify_demo_watcher_close(&watcher);
        return EXIT_FAILURE;
    }

    printf("Watching %s. Press Ctrl-C to stop.\n", argv[1]);

    while (!g_should_stop) {
        struct pollfd poll_fd = {
            .fd = watcher.fd,
            .events = POLLIN,
            .revents = 0,
        };

        /* poll avoids blocking forever so SIGINT/SIGTERM can stop the loop promptly. */
        const int ready = poll(&poll_fd, 1, 500);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("poll");
            inotify_demo_watcher_close(&watcher);
            return EXIT_FAILURE;
        }

        if (ready == 0) {
            continue;
        }

        if ((poll_fd.revents & POLLIN) != 0) {
            /* One readable notification can contain a batch of inotify events. */
            if (inotify_demo_watcher_dispatch(&watcher, print_event, NULL) < 0) {
                perror("inotify dispatch");
                inotify_demo_watcher_close(&watcher);
                return EXIT_FAILURE;
            }
        }
    }

    inotify_demo_watcher_close(&watcher);
    return EXIT_SUCCESS;
}
