/*
 * Use of the inotify APIs to monitor the updates to the content of a directory.
 *
 * Laurent Gauthier <laurent.gauthier@soccasys.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define BUFFER_LENGTH (1024*(EVENT_SIZE + 16))

int main(int argc, char **argv)
{
    int bytes_read;
    int file_descriptor;
    int watch_descriptor;
    char buffer[BUFFER_LENGTH];

    file_descriptor = inotify_init();
    if (file_descriptor < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    watch_descriptor = inotify_add_watch(file_descriptor, argv[1],
                         IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if (watch_descriptor < 0) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH);
    if (bytes_read < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    while (bytes_read > 0) {
        int bytes_processed = 0;

        while (bytes_processed < bytes_read) {
            struct inotify_event *event = (struct inotify_event *)&buffer[bytes_processed];

            if (event->len) {
                if (event->mask & IN_MOVED_FROM) {
                    if (event->mask & IN_ISDIR) {
                        printf("The directory %s was moved from (cookie: %d).\n", event->name, event->cookie);
                    } else {
                        printf("The file %s was moved from (cookie: %d).\n", event->name, event->cookie);
                    }
                } else if (event->mask & IN_MOVED_TO) {
                    if (event->mask & IN_ISDIR) {
                        printf("The directory %s was moved to (cookie: %d).\n", event->name, event->cookie);
                    } else {
                        printf("The file %s was moved to (cookie: %d).\n", event->name, event->cookie);
                    }
                } else if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        printf("The directory %s was created.\n", event->name);
                    } else {
                        printf("The file %s was created.\n", event->name);
                    }
                } else if (event->mask & IN_DELETE) {
                    if (event->mask & IN_ISDIR) {
                        printf("The directory %s was deleted.\n", event->name);
                    } else {
                        printf("The file %s was deleted.\n", event->name);
                    }
                } else if (event->mask & IN_MODIFY) {
                    if (event->mask & IN_ISDIR) {
                        printf("The directory %s was modified.\n", event->name);
                    } else {
                        printf("The file %s was modified.\n", event->name);
                    }
                }
            }
            bytes_processed += EVENT_SIZE + event->len;
        }

        bytes_read = read(file_descriptor, buffer, BUFFER_LENGTH);
        if (bytes_read < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
    }

    (void) inotify_rm_watch(file_descriptor, watch_descriptor);
    (void) close(file_descriptor);

    exit(EXIT_SUCCESS);
}
