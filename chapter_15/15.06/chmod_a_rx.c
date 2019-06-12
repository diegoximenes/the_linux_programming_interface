#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>

#define PROGRAM_NAME "chmod_a_rx"

void usage(int status) {
    printf("Usage: %s FILE_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mychmod(const char *pathname, mode_t mode) {
    int ret = chmod(pathname, mode);
    if (ret == -1) {
        error("chmod");
    }
}

void mystat(const char *path, struct stat *buf) {
    int ret = stat(path, buf);
    if (ret  == -1) {
        error("stat");
    }
}

void chmod_a_rx(const char *file_path) {
    struct stat s;
    mystat(file_path, &s);
    mychmod(file_path, s.st_mode | S_IRUSR | S_IRGRP | S_IROTH);
    mystat(file_path, &s);
    if (S_ISDIR(s.st_mode) ||
            (s.st_mode & S_IXUSR) ||
            (s.st_mode & S_IXGRP) ||
            (s.st_mode & S_IXOTH)) {
        mychmod(file_path, s.st_mode | S_IXUSR | S_IXGRP | S_IXOTH);
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *file_path = NULL;

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
        file_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    chmod_a_rx(file_path);

    exit(EXIT_SUCCESS);
}
