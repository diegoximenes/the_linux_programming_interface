#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#define PROGRAM_NAME "fchdir_performance"

void usage(int status) {
    printf("Usage: %s ITERATIONS DIR_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

int myopen(const char *pathname, int flags) {
    int fd = open(pathname, flags);
    if (fd == -1) {
        error("open");
    }
    return fd;
}

void myclose(int fd) {
    if (close(fd) == -1) {
        error("close");
    }
}

void mygetcwd(char *buf, size_t size) {
    if (getcwd(buf, size) == NULL) {
        error("getcwd");
    }
}

void myfchdir(int fildes) {
    if (fchdir(fildes) == -1) {
        error("chdir");
    }
}

void fchdir_performance(unsigned iterations, const char *dir_path) {
    int fd_cwd = myopen(".", O_RDONLY);
    int fd_new_cwd = myopen(dir_path, O_RDONLY);
    while (iterations--) {
        myfchdir(fd_new_cwd);
        myfchdir(fd_cwd);
    }
    myclose(fd_cwd);
    myclose(fd_new_cwd);
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

    fchdir_performance(iterations, dir_path);

    exit(EXIT_SUCCESS);
}
