#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ctype.h>
#include "writer_reader.h"

#define PROGRAM_NAME "reader"

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    // get shared memory related to semaphores
    sem_t *semaphore_writer = get_semaphore(SEMAPHORE_WRITER_NAME, O_RDWR);
    sem_t *semaphore_reader = get_semaphore(SEMAPHORE_READER_NAME, O_RDWR);

    // get shared memory related to buffer
    ssize_t *buffer_len = get_shm(BUFFER_LEN_NAME,
                                  O_RDONLY,
                                  sizeof(ssize_t),
                                  PROT_READ);
    char *buffer = get_shm(BUFFER_NAME,
                           O_RDONLY,
                           sizeof(char) * BUFFER_SIZE,
                           PROT_READ);


    while (1) {
        my_sem_wait(semaphore_reader);

        if (!*buffer_len) {
            break;
        }

        for (ssize_t i = 0; i < *buffer_len; ++i) {
            printf("%c", toupper(buffer[i]));
        }

        my_sem_post(semaphore_writer);
    }

    exit(EXIT_SUCCESS);
}
