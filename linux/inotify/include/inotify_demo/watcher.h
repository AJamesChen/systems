#ifndef INOTIFY_DEMO_WATCHER_H
#define INOTIFY_DEMO_WATCHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/inotify.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct inotify_demo_watcher {
    int fd;
} inotify_demo_watcher;

typedef int (*inotify_demo_event_callback)(const struct inotify_event *event, void *context);

int inotify_demo_watcher_init(inotify_demo_watcher *watcher, bool nonblocking);
int inotify_demo_watcher_add(inotify_demo_watcher *watcher, const char *path, uint32_t mask);
int inotify_demo_watcher_dispatch(inotify_demo_watcher *watcher,
                                  inotify_demo_event_callback callback,
                                  void *context);
void inotify_demo_watcher_close(inotify_demo_watcher *watcher);

size_t inotify_demo_format_mask(uint32_t mask, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
