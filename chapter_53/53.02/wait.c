#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "wait_post.h"

#define PROGRAM_NAME "wait"

void usage(int status) {
    printf("Usage: %s SEMAPHORE_NAME SECONDS_TO_WAIT\n", PROGRAM_NAME);
    exit(status);
}

void my_clock_gettime(clockid_t clock_id, struct timespec *tp) {
    if (clock_gettime(clock_id, tp) == -1) {
        error("clock_gettime");
    }
}

void my_sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
    if (sem_timedwait(sem, abs_timeout) == -1) {
        error("sem_timedwait");
    }
}

sem_t *my_sem_open(const char *name,
                   int oflag,
                   mode_t mode,
                   unsigned int value) {
    sem_t *ret = sem_open(name, oflag, mode, value);
    if (ret == SEM_FAILED) {
        error("sem_open");
    }
    return ret;
}

void my_sem_unlink(const char *name) {
    if (sem_unlink(name) == -1) {
        error("sem_unlink");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *semaphore_name = NULL;
    time_t seconds_to_wait = 0;

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
        semaphore_name = argv[optind++];
        seconds_to_wait = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    sem_t *semaphore = my_sem_open(semaphore_name, O_CREAT | O_EXCL, mode, 0);

    struct timespec t;
    my_clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec = t.tv_sec + seconds_to_wait;

    my_sem_timedwait(semaphore, &t);

    my_sem_unlink(semaphore_name);

    exit(EXIT_SUCCESS);
}
