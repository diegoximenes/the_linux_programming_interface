#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>

#define PROGRAM_NAME "init_parent"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

pid_t myfork() {
    pid_t pid = fork();
    if (pid == -1) {
        error("fork");
    }
    return pid;
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
    while ((opt = getopt_long(argc, argv, "a", long_options, NULL)) != -1) {
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

    pid_t pid = myfork();
    if (pid == 0) {
        sleep(5);
        pid_t ppid = getppid();
        printf("ppid=%d\n", ppid);
    }

    exit(EXIT_SUCCESS);
}
