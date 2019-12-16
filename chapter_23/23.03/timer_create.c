#define _POSIX_C_SOURCE 199309
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <time.h>

#define PROGRAM_NAME "timer_create"

int TIMER_ID = 12345;

void usage(int status) {
    printf("Usage: %s SECONDS\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mysigaction(int sig,
                const struct sigaction *act,
                struct sigaction *oact) {
    if (sigaction(sig, act, oact) == -1) {
        error("sigaction");
    }
}

void mysigemptyset(sigset_t *set) {
    if (sigemptyset(set) == -1) {
        error("sigemptyset");
    }
}

void mysigfillset(sigset_t *set) {
    if (sigfillset(set) == -1) {
        error("sigemptyset");
    }
}

void my_timer_create(clockid_t clockid,
                    struct sigevent *evp,
                    timer_t *timerid) {
    if (timer_create(clockid, evp, timerid) != 0) {
        error("timer_create");
    }
}

void my_timer_settime(timer_t timerid,
                      int flags,
                      const struct itimerspec *value,
                      struct itimerspec *ovalue) {
    if (timer_settime(timerid, flags, value, ovalue) != 0) {
        error("timer_settime");
    }
}

// this function is non reentrant only for didactic purposes
void sighandler(int sig, siginfo_t *si, void *uc) {
    // check that correct is 1
    printf("sighandler, sig=%d, correct=%d, si->si_value.sival_int=%d\n",
            sig, si->si_value.sival_int == TIMER_ID, si->si_value.sival_int);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    time_t alarm_seconds;

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
        alarm_seconds = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    // set signal handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    mysigemptyset(&sa.sa_mask);
    sa.sa_sigaction = &sighandler;
    mysigaction(SIGALRM, &sa, NULL);

    // create timer
    timer_t timer_id;
    struct sigevent evp;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = SIGALRM;
    evp.sigev_value.sival_int = TIMER_ID;
    my_timer_create(CLOCK_REALTIME, &evp, &timer_id);

    // set timer
    struct itimerspec timer_spec;
    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = 0;
    timer_spec.it_value.tv_sec = alarm_seconds;
    timer_spec.it_value.tv_nsec = 0;
    my_timer_settime(timer_id, 0, &timer_spec, NULL);

    while (1);

    exit(EXIT_SUCCESS);
}
