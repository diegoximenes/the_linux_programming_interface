#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "writer_reader.h"

#define PROGRAM_NAME "writer"

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
}

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
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
    sem_t *semaphore_writer =
        get_semaphore(SEMAPHORE_WRITER_NAME, O_CREAT | O_EXCL | O_RDWR);
    my_sem_init(semaphore_writer, 1, 1);

    sem_t *semaphore_reader =
        get_semaphore(SEMAPHORE_READER_NAME, O_CREAT | O_EXCL | O_RDWR);
    my_sem_init(semaphore_reader, 1, 0);

    // get shared memory related to buffer
    ssize_t *buffer_len = get_shm(BUFFER_LEN_NAME,
                                  O_CREAT | O_EXCL | O_RDWR,
                                  sizeof(ssize_t),
                                  PROT_READ | PROT_WRITE);

    char *buffer = get_shm(BUFFER_NAME,
                           O_CREAT | O_EXCL | O_RDWR,
                           sizeof(char) * BUFFER_SIZE,
                           PROT_READ | PROT_WRITE);

    while (1) {
        my_sem_wait(semaphore_writer);

        ssize_t cnt_read = myread(STDIN_FILENO, buffer, BUFFER_SIZE);

        *buffer_len = cnt_read;

        my_sem_post(semaphore_reader);
        if (!cnt_read) {
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
