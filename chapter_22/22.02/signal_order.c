#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#define PROGRAM_NAME "signal_order"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
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

void mysigprocmask(int how, const sigset_t *set, sigset_t *oset) {
    if (sigprocmask(how, set, oset) == -1) {
        error("sigprocmask");
    }
}

// this function is non reentrant only for didactic purposes
void default_sighandler(int sig) {
    printf("default sighandler %d\n", sig);
}

// this function is non reentrant only for didactic purposes
void realtime_sighandler(int sig) {
    printf("realtime sighandler %d\n", sig);
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

    // set default signal handler
    struct sigaction sa;
    sa.sa_flags = 0;
    mysigemptyset(&sa.sa_mask);
    sa.sa_handler = &default_sighandler;
    mysigaction(2, &sa, NULL);

    // set realtime signal handler
    sa.sa_handler = &realtime_sighandler;
    mysigaction(60, &sa, NULL);

    // block all signals
    sigset_t all_signals;
    mysigfillset(&all_signals);
    mysigprocmask(SIG_BLOCK, &all_signals, NULL);

    printf("before sleep\n");
    sleep(40);

    // unblock all signals
    mysigfillset(&all_signals);
    mysigprocmask(SIG_UNBLOCK, &all_signals, NULL);

    exit(EXIT_SUCCESS);
}
