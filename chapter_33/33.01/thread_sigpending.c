#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define PROGRAM_NAME "thread_sigpending"

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mysigpending(sigset_t *set) {
    if (sigpending(set) == -1) {
        error("sigpending");
    }
}

void mysigfillset(sigset_t *set) {
    if (sigfillset(set) == -1) {
        error("sigemptyset");
    }
}

void mysigprocmask(int how, const sigset_t *set, sigset_t *oset) {
    if (sigprocmask(how, set, oset) == -1) {
        error("sigprocmask");
    }
}

int mysigismember(const sigset_t *set, int signum) {
    int ret = sigismember(set, signum);
    if (ret == -1) {
        error("sigismember");
    }
    return ret;
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

void my_pthread_kill(pthread_t thread, int sig) {
    int ret = pthread_kill(thread, sig);
    if (ret != 0) {
        errno = ret;
        error("pthread_kill");
    }
}

pthread_t thread1, thread2;
pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_f(void *arg) {
    // wait so that main thread is able to send signals
    sleep(1);

    // get pending signals
    sigset_t pending_signals;
    mysigpending(&pending_signals);

    // get thread name
    pthread_t current_thread = pthread_self();
    char thread_name[8];
    if (pthread_equal(thread1, current_thread)) {
        strcpy(thread_name, "thread1");
    } else if (pthread_equal(thread2, current_thread)) {
        strcpy(thread_name, "thread2");
    } else {
        strcpy(thread_name, "error");
    }

    // print pending signals
    my_pthread_mutex_lock(&stdout_mutex);
    for (int sig = 1; sig < NSIG; ++sig) {
        if (mysigismember(&pending_signals, sig)) {
            printf("%s: %d\n", thread_name, sig);
        }
    }
    my_pthread_mutex_unlock(&stdout_mutex);

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

    // block all signals
    sigset_t all_signals;
    mysigfillset(&all_signals);
    mysigprocmask(SIG_BLOCK, &all_signals, NULL);

    // create threads
    my_pthread_create(&thread1, NULL, thread_f, NULL);
    my_pthread_create(&thread2, NULL, thread_f, NULL);

    // send signals to threads
    my_pthread_kill(thread1, 4);
    my_pthread_kill(thread1, 5);
    my_pthread_kill(thread2, 6);
    my_pthread_kill(thread2, 7);
    my_pthread_kill(thread2, 8);

    // wait threads
    my_pthread_join(thread1, NULL);
    my_pthread_join(thread2, NULL);

    exit(EXIT_SUCCESS);
}
