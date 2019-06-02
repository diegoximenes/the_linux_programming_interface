#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>

#define PROGRAM_NAME "setfattr"

void usage(int status) {
    printf("Usage: %s FILE NAME VALUE\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void setfattr(const char *file_path, const char *ea_name, const char *ea_val) {
    int ret = setxattr(file_path, ea_name, ea_val, strlen(ea_val), 0);
    if (ret == -1) {
        error("setxattr");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *file_path = NULL, *ea_name = NULL, *ea_val = NULL;

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
    if (optind == argc - 3) {
        file_path = argv[optind++];
        ea_name = argv[optind++];
        ea_val = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    setfattr(file_path, ea_name, ea_val);

    exit(EXIT_SUCCESS);
}
