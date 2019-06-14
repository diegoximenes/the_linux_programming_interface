#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>

#define PROGRAM_NAME "chdir_performance"

void usage(int status) {
    printf("Usage: %s ITERATIONS DIR_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mygetcwd(char *buf, size_t size) {
    if (getcwd(buf, size) == NULL) {
        error("getcwd");
    }
}

void mychdir(const char *path) {
    if (chdir(path) == -1) {
        error("chdir");
    }
}

void chdir_performance(unsigned iterations, const char *dir_path) {
    char cwd[PATH_MAX];
    mygetcwd(cwd, PATH_MAX);
    while (iterations--) {
        mychdir(dir_path);
        mychdir(cwd);
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt, iterations = 0;
    char *dir_path = NULL;

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
    if (optind == argc - 2) {
        iterations = atoi(argv[optind++]);
        dir_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    chdir_performance(iterations, dir_path);

    exit(EXIT_SUCCESS);
}
