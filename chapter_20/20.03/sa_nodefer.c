#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>

#define PROGRAM_NAME "sa_nodefer"

void usage(int status) {
    printf("Usage: %s USE_NODEFER (0 or 1)\n", PROGRAM_NAME);
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

// this function is non reentrant only for didactic purposes
void sighandler(int sig) {
    printf("sighandler %d\n", sig);
    while (1);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    int nodefer = 0;

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
        nodefer = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    // set sighandler
    struct sigaction sa;
    mysigaction(SIGINT, NULL, &sa);
    sa.sa_handler = &sighandler;
    if (nodefer) {
        sa.sa_flags |= SA_NODEFER;
    }
    mysigaction(SIGINT, &sa, NULL);

    while (1);

    exit(EXIT_SUCCESS);
}
