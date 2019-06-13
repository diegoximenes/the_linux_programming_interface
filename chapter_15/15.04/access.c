#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#define PROGRAM_NAME "access"

void usage(int status) {
    printf("Usage: %s FILE_PATH\n", PROGRAM_NAME);
    exit(status);
}

int access_effective(const char *path, int mode) {
    return faccessat(AT_FDCWD, path, mode, AT_EACCESS);
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

    int ret_x = access_effective(file_path, X_OK);
    int ret_r = access_effective(file_path, R_OK);
    printf("X_OK=%d, R_OK=%d\n", ret_x, ret_r);

    exit(EXIT_SUCCESS);
}
