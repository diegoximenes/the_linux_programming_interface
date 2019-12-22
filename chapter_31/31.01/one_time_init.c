#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>

#define PROGRAM_NAME "one_time_init"

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void my_pthread_mutex_lock(pthread_mutex_t *mutex) {
    int ret = pthread_mutex_lock(mutex);
    if (ret != 0) {
        errno = ret;
        error("pthread_mutex_lock");
    }
}

void my_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
        errno = ret;
        error("pthread_mutex_unlock");
    }
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

struct one_time_init_control {
    int initialized;
    pthread_mutex_t mutex;
};

#define ONE_TIME_INIT_CONTROL_INITIALIZER { 0, PTHREAD_MUTEX_INITIALIZER }

void one_time_init(struct one_time_init_control *control, void (*init)(void)) {
    my_pthread_mutex_lock(&control->mutex);
    if (!control->initialized) {
        init();
        control->initialized = 1;
    }
    my_pthread_mutex_unlock(&control->mutex);
}

void my_init() {
    printf("my_init\n");
}

struct one_time_init_control process_control =
    ONE_TIME_INIT_CONTROL_INITIALIZER;

void *thread_f(void *arg) {
    for (int i = 0; i < 100; ++i) {
        one_time_init(&process_control, my_init);
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

    pthread_t thread1, thread2;

    my_pthread_create(&thread1, NULL, thread_f, NULL);
    my_pthread_create(&thread2, NULL, thread_f, NULL);

    my_pthread_join(thread1, NULL);
    my_pthread_join(thread2, NULL);

    exit(EXIT_SUCCESS);
}
