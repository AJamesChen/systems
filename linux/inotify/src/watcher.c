#include "inotify_demo/watcher.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* inotify returns one or more variable-length records from each read. */
#define EVENT_BUFFER_SIZE (16U * (sizeof(struct inotify_event) + 256U))

typedef struct mask_name {
    uint32_t mask;
    const char *name;
} mask_name;

static const mask_name k_mask_names[] = {
    {IN_ACCESS, "ACCESS"},
    {IN_ATTRIB, "ATTRIB"},
    {IN_CLOSE_WRITE, "CLOSE_WRITE"},
    {IN_CLOSE_NOWRITE, "CLOSE_NOWRITE"},
    {IN_CREATE, "CREATE"},
    {IN_DELETE, "DELETE"},
    {IN_DELETE_SELF, "DELETE_SELF"},
    {IN_MODIFY, "MODIFY"},
    {IN_MOVE_SELF, "MOVE_SELF"},
    {IN_MOVED_FROM, "MOVED_FROM"},
    {IN_MOVED_TO, "MOVED_TO"},
    {IN_OPEN, "OPEN"},
    {IN_IGNORED, "IGNORED"},
    {IN_ISDIR, "ISDIR"},
    {IN_Q_OVERFLOW, "Q_OVERFLOW"},
    {IN_UNMOUNT, "UNMOUNT"},
};

int inotify_demo_watcher_init(inotify_demo_watcher *watcher, bool nonblocking)
{
    if (watcher == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* IN_CLOEXEC prevents leaking this fd into child processes after exec. */
    int flags = IN_CLOEXEC;
    if (nonblocking) {
        flags |= IN_NONBLOCK;
    }

    const int fd = inotify_init1(flags);
    if (fd < 0) {
        return -1;
    }

    watcher->fd = fd;
    return 0;
}

int inotify_demo_watcher_add(inotify_demo_watcher *watcher, const char *path, uint32_t mask)
{
    if (watcher == NULL || watcher->fd < 0 || path == NULL) {
        errno = EINVAL;
        return -1;
    }

    return inotify_add_watch(watcher->fd, path, mask);
}

int inotify_demo_watcher_dispatch(inotify_demo_watcher *watcher,
                                  inotify_demo_event_callback callback,
                                  void *context)
{
    if (watcher == NULL || watcher->fd < 0 || callback == NULL) {
        errno = EINVAL;
        return -1;
    }

    /*
     * The buffer must be suitably aligned because it is walked as a sequence of
     * struct inotify_event records followed by optional filename bytes.
     */
    char buffer[EVENT_BUFFER_SIZE] __attribute__((aligned(__alignof__(struct inotify_event))));
    const ssize_t bytes_read = read(watcher->fd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        /* A nonblocking watcher reports "no events available" as EAGAIN. */
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }

    int events_seen = 0;
    for (char *ptr = buffer; ptr < buffer + bytes_read;) {
        const struct inotify_event *event = (const struct inotify_event *)ptr;
        if (callback(event, context) != 0) {
            return -1;
        }

        events_seen++;
        /* event->len is the padded length of the optional name field. */
        ptr += sizeof(struct inotify_event) + event->len;
    }

    return events_seen;
}

void inotify_demo_watcher_close(inotify_demo_watcher *watcher)
{
    if (watcher == NULL || watcher->fd < 0) {
        return;
    }

    (void)close(watcher->fd);
    watcher->fd = -1;
}

size_t inotify_demo_format_mask(uint32_t mask, char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0U) {
        return 0U;
    }

    size_t used = 0U;
    buffer[0] = '\0';

    /* A single inotify event can carry several mask bits at once. */
    for (size_t i = 0U; i < sizeof(k_mask_names) / sizeof(k_mask_names[0]); ++i) {
        if ((mask & k_mask_names[i].mask) == 0U) {
            continue;
        }

        const char *separator = used == 0U ? "" : "|";
        const int written = snprintf(buffer + used,
                                     buffer_size - used,
                                     "%s%s",
                                     separator,
                                     k_mask_names[i].name);
        if (written < 0) {
            return used;
        }

        const size_t written_size = (size_t)written;
        if (written_size >= buffer_size - used) {
            buffer[buffer_size - 1U] = '\0';
            return buffer_size - 1U;
        }

        used += written_size;
    }

    /* Keep unknown masks visible for debugging instead of returning an empty string. */
    if (used == 0U) {
        const int written = snprintf(buffer, buffer_size, "0x%08x", mask);
        if (written < 0) {
            buffer[0] = '\0';
            return 0U;
        }

        const size_t written_size = (size_t)written;
        return written_size < buffer_size ? written_size : buffer_size - 1U;
    }

    return used;
}
