#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define PROGRAM_NAME "rusage"

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

pid_t mywait(int *status) {
    pid_t pid = wait(status);
    if (pid == -1) {
        error("wait");
    }
    return pid;
}

void myexecvp(const char *pathname, char *const argv[]) {
    if (execvp(pathname, argv) == -1) {
        error("execv");
    }
}

void mygetrusage(int who, struct rusage *r_usage) {
    if (getrusage(who, r_usage) == -1) {
        error("getrusage");
    }
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

    pid_t pid_first_fork = myfork();
    if (pid_first_fork == 0) {
        myexecvp(argv[1], &argv[1]);
    } else {
        mywait(NULL);

        struct rusage r_usage;
        mygetrusage(RUSAGE_CHILDREN, &r_usage);

        double user_time =
            r_usage.ru_utime.tv_sec + r_usage.ru_utime.tv_usec / 1000000.0;
        double system_time =
            r_usage.ru_stime.tv_sec + r_usage.ru_stime.tv_usec / 1000000.0;
        printf("user_time=%.2lf, cpu_time=%.2lf\n", user_time, system_time);
    }

    exit(EXIT_SUCCESS);
}
