#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <ctype.h>

#define PROGRAM_NAME "writer_reader"

#define BUFFER_SIZE 4096

ssize_t buffer_size;
char buffer[BUFFER_SIZE];
sem_t semaphore_writer, semaphore_reader;

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void my_sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (sem_init(sem, pshared, value) == -1) {
        error("sem_init");
    }
}

void my_sem_wait(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        error("sem_wait");
    }
}

void my_sem_post(sem_t *sem) {
    if (sem_post(sem) == -1) {
        error("sem_wait");
    }
}

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
}

void my_pthread_join(pthread_t thread, void **retval) {
    int ret = pthread_join(thread, retval);
    if (ret != 0) {
        errno = ret;
        error("pthread_join");
    }
}

void my_pthread_create(pthread_t *thread,
                       const pthread_attr_t *attr,
                       void *(*start_routine) (void *),
                       void *arg) {
    int ret = pthread_create(thread, attr, start_routine, arg);
    if (ret != 0) {
        errno = ret;
        error("pthread_create");
    }
}

void *thread_writer_f(void *arg) {
    while (1) {
        my_sem_wait(&semaphore_writer);

        ssize_t cnt_read = myread(STDIN_FILENO, buffer, BUFFER_SIZE);
        buffer_size = cnt_read;

        my_sem_post(&semaphore_reader);
        if (!cnt_read) {
            break;
        }
    }

    return NULL;
}

void *thread_reader_f(void *arg) {
    while (1) {
        my_sem_wait(&semaphore_reader);

        if (!buffer_size) {
            break;
        }

        for (ssize_t i = 0; i < buffer_size; ++i) {
            printf("%c", toupper(buffer[i]));
        }

        my_sem_post(&semaphore_writer);
    }

    return NULL;
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

    // init semaphores
    my_sem_init(&semaphore_writer, 0, 1);
    my_sem_init(&semaphore_reader, 0, 0);

    // create threads
    pthread_t thread_writer, thread_reader;
    my_pthread_create(&thread_writer, NULL, thread_writer_f, NULL);
    my_pthread_create(&thread_reader, NULL, thread_reader_f, NULL);

    // wait threads
    my_pthread_join(thread_writer, NULL);
    my_pthread_join(thread_reader, NULL);

    exit(EXIT_SUCCESS);
}
