#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define PROGRAM_NAME "getrusage_children"

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

void mygetrusage(int who, struct rusage *r_usage) {
    if (getrusage(who, r_usage) == -1) {
        error("getrusage");
    }
}

double rusage_time(const struct rusage *r_usage) {
    double time =
        r_usage->ru_utime.tv_sec + r_usage->ru_utime.tv_sec / 1000000.0;
    time += r_usage->ru_stime.tv_sec + r_usage->ru_stime.tv_sec / 1000000.0;
    return time;
}

void use_cpu_time() {
    int tmp = 0;
    for (int i = 0; i < 1000000000; ++i) {
        tmp = (tmp + 1) % 10;
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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    pid_t pid_first_fork = myfork();
    if (pid_first_fork == 0) {
        // child 1
        use_cpu_time();
        printf("child1 terminating\n");
    } else {
        pid_t pid_second_fork = myfork();
        if (pid_second_fork == 0) {
            use_cpu_time();
            printf("child2 terminating\n");
        } else {
            // parent

            // wait to children become zombies
            sleep(10);

            // usage before wait
            struct rusage r_usage;
            mygetrusage(RUSAGE_CHILDREN, &r_usage);
            printf("before wait: time=%lf\n", rusage_time(&r_usage));

            mywait(NULL);

            // usage after first wait
            mygetrusage(RUSAGE_CHILDREN, &r_usage);
            printf("after first wait: time=%lf\n", rusage_time(&r_usage));

            mywait(NULL);

            // usage after second wait
            mygetrusage(RUSAGE_CHILDREN, &r_usage);
            printf("after second wait: time=%lf\n", rusage_time(&r_usage));
        }
    }

    exit(EXIT_SUCCESS);
}
