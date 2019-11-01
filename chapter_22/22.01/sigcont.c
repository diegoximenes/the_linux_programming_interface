#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>

#define PROGRAM_NAME "sigcont"

void usage(int status) {
    printf("Usage: %s BLOCK_SIGCONT (0 or 1)\n", PROGRAM_NAME);
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

void mysigaddset(sigset_t *set, int signum) {
    if (sigaddset(set, signum) == -1) {
        error("sigaddset");
    }
}

void mysigprocmask(int how, const sigset_t *set, sigset_t *oset) {
    if (sigprocmask(how, set, oset) == -1) {
        error("sigprocmask");
    }
}

// this function is non reentrant only for didactic purposes
void sighandler(int sig) {
    printf("sighandler %d\n", sig);
}

void block_signal(int sig) {
    sigset_t block_set;
    mysigemptyset(&block_set);
    mysigaddset(&block_set, sig);
    mysigprocmask(SIG_BLOCK, &block_set, NULL);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    int block_sigcont = 0;

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
        block_sigcont = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    if (block_sigcont) {
        block_signal(SIGCONT);
    }

    // set handler
    struct sigaction sa;
    sa.sa_handler = &sighandler;
    mysigaction(SIGCONT, &sa, NULL);

    while (1);

    exit(EXIT_SUCCESS);
}
