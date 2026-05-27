#include "inotify_demo/watcher.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define TEST_TIMEOUT_MS 2000

typedef struct observed_event {
    uint32_t mask;
    char name[256];
} observed_event;

static void require_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static int capture_create_event(const struct inotify_event *event, void *context)
{
    observed_event *observed = (observed_event *)context;
    /* Ignore incidental events and keep the assertion focused on IN_CREATE. */
    if ((event->mask & IN_CREATE) != 0U) {
        observed->mask = event->mask;
        if (event->len > 0U) {
            (void)snprintf(observed->name, sizeof(observed->name), "%s", event->name);
        }
    }

    return 0;
}

static void test_format_mask(void)
{
    char buffer[128];
    const size_t written = inotify_demo_format_mask(IN_CREATE | IN_ISDIR, buffer, sizeof(buffer));

    require_true(written > 0U, "format_mask reports bytes written");
    require_true(strstr(buffer, "CREATE") != NULL, "format_mask includes CREATE");
    require_true(strstr(buffer, "ISDIR") != NULL, "format_mask includes ISDIR");
}

static void test_format_mask_truncates(void)
{
    char buffer[8];
    const size_t written = inotify_demo_format_mask(IN_CREATE | IN_CLOSE_WRITE, buffer, sizeof(buffer));

    require_true(written == sizeof(buffer) - 1U, "format_mask reports truncated length");
    require_true(buffer[sizeof(buffer) - 1U] == '\0', "format_mask null terminates truncated output");
}

static void test_watch_directory_create_event(void)
{
    char temp_dir[] = "/tmp/inotify-demo-test-XXXXXX";
    require_true(mkdtemp(temp_dir) != NULL, "mkdtemp creates test directory");

    /* Register the watch before creating the file so the create event is observable. */
    inotify_demo_watcher watcher = {.fd = -1};
    require_true(inotify_demo_watcher_init(&watcher, true) == 0, "watcher initializes");
    require_true(inotify_demo_watcher_add(&watcher, temp_dir, IN_CREATE) >= 0, "watcher adds directory");

    char file_path[512];
    const int path_len = snprintf(file_path, sizeof(file_path), "%s/created.txt", temp_dir);
    require_true(path_len > 0 && (size_t)path_len < sizeof(file_path), "test file path fits");

    const int fd = open(file_path, O_CREAT | O_WRONLY | O_CLOEXEC, 0600);
    require_true(fd >= 0, "test file is created");
    require_true(close(fd) == 0, "test file is closed");

    observed_event observed = {.mask = 0U, .name = {0}};
    struct pollfd poll_fd = {
        .fd = watcher.fd,
        .events = POLLIN,
        .revents = 0,
    };

    /* Wait for readiness before dispatching so the nonblocking read does not race. */
    const int ready = poll(&poll_fd, 1, TEST_TIMEOUT_MS);
    require_true(ready > 0, "inotify fd becomes readable");
    require_true((poll_fd.revents & POLLIN) != 0, "inotify fd reports POLLIN");
    require_true(inotify_demo_watcher_dispatch(&watcher, capture_create_event, &observed) > 0,
                 "dispatch reads at least one event");
    require_true((observed.mask & IN_CREATE) != 0U, "observed IN_CREATE");
    require_true(strcmp(observed.name, "created.txt") == 0, "observed created filename");

    inotify_demo_watcher_close(&watcher);
    require_true(unlink(file_path) == 0, "test file is removed");
    require_true(rmdir(temp_dir) == 0, "test directory is removed");
}

int main(void)
{
    test_format_mask();
    test_format_mask_truncates();
    test_watch_directory_create_event();

    puts("PASS: inotify demo tests");
    return EXIT_SUCCESS;
}
