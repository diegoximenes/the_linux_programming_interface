#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROGRAM_NAME "grandparent_parent_child"

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

    // used to check if process is grandparent, parent or child
    int id;

    pid_t pid_first_fork = myfork();
    if (pid_first_fork == 0) {
        // parent

        pid_t pid_second_fork = myfork();
        if (pid_second_fork == 0) {
            id = 2;
            // child
            sleep(1);
            pid_t ppid = getppid();
            printf("child ppid=%d\n", ppid);
            sleep(3);
            ppid = getppid();
            printf("child ppid=%d\n", ppid);
        } else {
            //parent
            id = 1;
        }
    } else {
        // grandparent
        id = 0;
        sleep(2);
        // wait for parent
        mywait(NULL);
        printf("grandparent waited\n");
        sleep(6);
    }

    switch (id) {
        case 0:
            printf("grandparent exiting\n");
            break;
        case 1:
            printf("parent exiting\n");
            break;
        case 2:
            printf("child exiting\n");
            break;
    }

    exit(EXIT_SUCCESS);
}
