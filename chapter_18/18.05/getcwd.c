#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#define PROGRAM_NAME "getcwd"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
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

char *mygetcwd(char *buf, size_t size) {
    struct stat last_dir_status;
    if (stat(".", &last_dir_status) == -1) {
        return NULL;
    }

    size_t cwd_len = 0;
    while (1) {
        if (chdir("..") == -1) {
            return NULL;
        }

        // last dir is '/', therefore chdir("..") will maintain dir in '/'.
        // stop search after reaching '/'
        struct stat curr_dir_status;
        if (stat(".", &curr_dir_status) == -1) {
            return NULL;
        }
        if (curr_dir_status.st_ino == last_dir_status.st_ino) {
            break;
        }

        DIR *dir = opendir(".");
        if (dir == NULL) {
            return NULL;
        }

        // iterate through files of the current dir
        while (1) {
            struct dirent *dir_entry = readdir(dir);
            errno = 0;
            if (dir_entry == NULL) {
                if (errno != 0) {
                    return NULL;
                }
                break;
            }

            // check if this file inode is the same as the file inode of the
            // previous dir
            if (last_dir_status.st_ino == dir_entry->d_ino) {
                // check if this entry fits in buf
                size_t dir_entry_name_len = strlen(dir_entry->d_name);
                if (1 + cwd_len + dir_entry_name_len + 1 > size) {
                    errno = ERANGE;
                    return NULL;
                }

                // add this entry to buf.
                // always add to the end of the array to avoid using a stack
                size_t buf_entry_start =
                    size - 1 - cwd_len - dir_entry_name_len - 1;
                buf[buf_entry_start] = '/';
                for (size_t i = 0; i < dir_entry_name_len; ++i) {
                    buf[buf_entry_start + 1 + i] = dir_entry->d_name[i];
                }
                cwd_len += 1 + dir_entry_name_len;

                if (stat(".", &last_dir_status) == -1) {
                    return NULL;
                }
                break;
            }
        }
    }

    // shift buf to the left
    size_t buf_start = size - 1 - cwd_len;
    for (size_t i = buf_start; i < size; ++i) {
        buf[i - buf_start] = buf[i];
    }
    buf[cwd_len] = 0;

    // cwd is '/'
    if (cwd_len == 0) {
        if (size < 2) {
            errno = ERANGE;
            return NULL;
        }
        buf[0] = '/';
        buf[1] = 0;
    }

    return buf;
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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    char *cwd = (char *) mymalloc(PATH_MAX);
    mygetcwd(cwd, PATH_MAX);
    if (cwd != NULL) {
        printf("%s\n", cwd);
    } else {
        error("mygetcwd");
    }

    exit(EXIT_SUCCESS);
}
