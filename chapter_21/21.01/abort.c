#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>

#define PROGRAM_NAME "abort"

void usage(int status) {
    printf("Usage: %s ADD_SIG_HANDLER (0 or 1)\n", PROGRAM_NAME);
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

void myraise(int sig) {
    if (raise(sig) != 0) {
        // it is not clear if errno is set in case of errors
        fprintf(stderr, "ERROR: raise\n");
        exit(EXIT_FAILURE);
    }
}

// this function is non reentrant only for didactic purposes
void sighandler(int sig) {
    printf("sighandler %d\n", sig);
}

void myabort() {
    raise(SIGABRT);
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    mysigaction(SIGABRT, &sa, NULL);
    raise(SIGABRT);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    int add_sig_handler = 0;

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
        add_sig_handler = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    if (add_sig_handler) {
        struct sigaction sa;
        sa.sa_handler = &sighandler;
        mysigaction(SIGABRT, &sa, NULL);
    }
    myabort();

    while (1);

    exit(EXIT_SUCCESS);
}
