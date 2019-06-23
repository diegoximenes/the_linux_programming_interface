// TODO: rename functionality

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <ftw.h>
#include <unordered_map>

#define PROGRAM_NAME "monitor_events"

#define UNUSED(x) (void)(x)

#define INOTIFY_BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int inotify_fd;
std::unordered_map<int, char*> wd_to_path; // stores directories beeing watched
char *dir_path = NULL; // program input

void usage(int status) {
    printf("Usage: %s DIR_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
}

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if ((size > 0) && (p == NULL)) {
        error("malloc");
    }
    return p;
}

void mystat(const char *pathname, struct stat *statbuf) {
    if (stat(pathname, statbuf) == -1) {
        error("stat");
    }
}

int my_inotify_init() {
    int ret = inotify_init();
    if (ret == -1) {
        error("inotify_init");
    }
    return ret;
}

int my_inotify_add_watch(int fd, const char *pathname, uint32_t mask) {
    int ret = inotify_add_watch(fd, pathname, mask);
    if (ret == -1) {
        error("inotify_add_watch");
    }
    return ret;
}

void mynftw(const char *dirpath,
            int (*fn) (const char *fpath,
                       const struct stat *sb,
                       int typeflag,
                       struct FTW *ftwbuf),
            int nopenfd,
            int flags) {
    if (nftw(dirpath, fn, nopenfd, flags) == -1) {
        error("nftw");
    }
}

int watch_dir(const char *pathname,
                const struct stat *statbuf,
                int typeflag,
                struct FTW *ftwbuf) {
    UNUSED(typeflag);
    UNUSED(ftwbuf);

    if (S_ISDIR(statbuf->st_mode)) {
        uint32_t mask = IN_CREATE | IN_DELETE;
        int wd = my_inotify_add_watch(inotify_fd, pathname, mask);

        char *path = (char *) mymalloc(strlen(pathname));
        strcpy(path, pathname);
        wd_to_path[wd] = path;

        printf("%15s %s\n", "[watching]", path);
    }

    return 0;
}

int is_dir(const char *path) {
    // if file created is a directory then watch it
    struct stat statbuf;
    mystat(path, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        return 1;
    }
    return 0;
}

void monitor_events() {
    inotify_fd = my_inotify_init();
    mynftw(dir_path, watch_dir, 10, 0);

    char *inotify_buf = (char *) mymalloc(INOTIFY_BUF_LEN);
    while (1) {
        ssize_t cnt_read = myread(inotify_fd, inotify_buf, INOTIFY_BUF_LEN);
        if (cnt_read == 0) {
            fprintf(stderr, "ERROR: inotify read returned 0\n");
            exit(EXIT_FAILURE);
        }

        // parse events
        for (char *p = inotify_buf; p < inotify_buf + cnt_read; ) {
            struct inotify_event *event = (struct inotify_event *) p;
            p += sizeof(struct inotify_event) + event->len;

            // ignore unwanted events
            if (!(event->mask & IN_CREATE) &&
                    !(event->mask & IN_DELETE) &&
                    !(event->mask & IN_IGNORED)) {
                continue;
            }

            // get file path of the event
            size_t prefix_len = strlen(wd_to_path[event->wd]);
            size_t file_name_len = strlen(event->name);
            size_t path_len = prefix_len + 1 + file_name_len + 1;
            char *path = (char *) mymalloc(path_len);
            strcpy(path, wd_to_path[event->wd]);
            path[prefix_len] = '/';
            strcpy(path + prefix_len + 1, event->name);

            if (event->mask & IN_CREATE) {
                printf("%15s %s\n", "[create]", path);

                // if file created is a directory then watch it
                if (is_dir(path)) {
                    mynftw(path, watch_dir, 10, 0);
                }
            } else if (event->mask & IN_DELETE) {
                printf("%15s %s\n", "[delete]", path);
            } else if (event->mask & IN_IGNORED) {
                printf("%15s %s\n", "[unwatching]", wd_to_path[event->wd]);
                wd_to_path.erase(event->wd);
            }

            free(path);
        }
    }

    // free wd_to_path paths
    std::unordered_map<int, char*>::iterator it;
    for (it = wd_to_path.begin(); it != wd_to_path.end(); ++it) {
        free(it->second);
    }

    free(inotify_buf);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (opt) {
            case GETOPT_HELP_CHAR:
                usage(EXIT_SUCCESS);
                break;
            default:
                usage(EXIT_FAILURE);
                break;
        }
    }
    if (optind == argc - 1) {
        dir_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    monitor_events();

    exit(EXIT_SUCCESS);
}
