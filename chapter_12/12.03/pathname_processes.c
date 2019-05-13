// should be executed with sudo privileges

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define PROGRAM_NAME "pathname_processes"

void usage(int status) {
    printf("Usage: %s USER_NAME\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if ((size > 0) && (p == NULL)) {
        error("malloc");
    }
    return p;
}

int valid_integer(const char *s) {
    char *endptr = NULL;
    errno = 0;
    strtol(s, &endptr, 10);
    if ((s == endptr) || (errno != 0)) {
        return 0;
    }
    return 1;
}

DIR *myopendir(const char *path, int exit_on_error) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        if ((exit_on_error == 1) || (errno != ENOENT)) {
            error("opendir");
        }
    }
    return dir;
}

struct dirent *myreaddir(DIR *dir, int exit_on_error) {
    errno = 0;
    struct dirent *dir_entry = readdir(dir);
    if (dir_entry == NULL) {
        if ((errno != 0) && (exit_on_error == 1)) {
            error("readdir");
        }
    }
    return dir_entry;
}

void myreadlink(const char *pathname,
                char *buf,
                size_t bufsize,
                int exit_on_error) {
    ssize_t buf_len = readlink(pathname, buf, bufsize);
    if (buf_len == -1) {
        if ((exit_on_error == 1) ||
                ((errno != ENOENT) && (errno != ENOTDIR))) {
            error("readlink");
        }
    } else if ((size_t) buf_len == bufsize) {
        errno = ENOMEM;
        error("readlink");
    } else {
        buf[buf_len] = '\0';
    }
}

int has_pathname_open(const char *pid, const char *pathname) {
    // build fd dir path
    char *fd_dir_path = (char *) mymalloc(6 + strlen(pid) + 4 + 1);
    strcpy(fd_dir_path, "/proc/");
    strcpy(fd_dir_path + 6, pid);
    strcpy(fd_dir_path + strlen(fd_dir_path), "/fd/");

    DIR *dir = myopendir(fd_dir_path, 0);
    if (dir == NULL) {
        return 0;
    }

    size_t readlink_buf_size = 4096;
    char *readlink_buf = (char *) mymalloc(readlink_buf_size);
    int has_pathname = 0;

    // iterate through open fds
    while (1) {
        struct dirent *dir_entry = myreaddir(dir, 0);
        if (dir_entry == NULL) {
            break;
        }

        char *fd = dir_entry->d_name;
        // check if it is a valid fd
        if (valid_integer(fd)) {
            size_t fd_path_len = strlen(fd_dir_path) + strlen(fd) + 1;
            char *fd_path = (char *) mymalloc(fd_path_len);
            strcpy(fd_path, fd_dir_path);
            strcpy(fd_path + strlen(fd_path), fd);

            // check if fd is related to pathname
            myreadlink(fd_path, readlink_buf, readlink_buf_size, 0);
            if (strcmp(readlink_buf, pathname) == 0) {
                has_pathname = 1;
                break;
            }
        }
    }

    free(fd_dir_path);
    free(readlink_buf);

    return has_pathname;
}

void pathname_processes(const char *pathname) {
    DIR *dir = myopendir("/proc/", 1);

    // iterate through processes
    while (1) {
        struct dirent *dir_entry = myreaddir(dir, 1);
        if (dir_entry == NULL) {
            break;
        }

        char *s_pid = dir_entry->d_name;
        // check if it is a valid pid
        if (valid_integer(s_pid)) {
            if (has_pathname_open(s_pid, pathname)) {
                printf("pid=%s\n", s_pid);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // opt vars
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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    const char *p = "/home/diego/Documents/dotfiles/config/polybar/launch.sh";
    pathname_processes(p);

    exit(EXIT_SUCCESS);
}
