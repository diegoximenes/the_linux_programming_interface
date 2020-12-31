#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <semaphore.h>
#include "wait_post.h"

#define PROGRAM_NAME "post"

void usage(int status) {
    printf("Usage: %s SEMAPHORE_NAME", PROGRAM_NAME);
    exit(status);
}

sem_t *my_sem_open(const char *name, int oflag) {
    sem_t *ret = sem_open(name, oflag);
    if (ret == SEM_FAILED) {
        error("sem_open");
    }
    return ret;
}

void my_sem_post(sem_t *sem) {
    if (sem_post(sem) == -1) {
        error("sem_post");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *semaphore_name = NULL;

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
        semaphore_name = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    sem_t *semaphore = my_sem_open(semaphore_name, 0);
    my_sem_post(semaphore);

    exit(EXIT_SUCCESS);
}
