#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define PROGRAM_NAME "siginterrupt"

#define BUFFER_SIZE 4096

void usage(int status) {
    printf("Usage: %s SIG FLAG\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    printf("cnt_read=%ld\n", cnt_read);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
}

void mysigaction(int sig,
                const struct sigaction *act,
                struct sigaction *oact) {
    if (sigaction(sig, act, oact) == -1) {
        error("sigaction");
    }
}

int mysiginterrupt(int sig, int flag) {
    struct sigaction sa;
    if (sigaction(sig, NULL, &sa) == -1) {
        if (errno == EINVAL) {
            return -1;
        }
        error("sigaction");
    }

    if (!flag) {
        sa.sa_flags |= SA_RESTART;
    } else {
        sa.sa_flags &= ~SA_RESTART;
    }

    if (sigaction(sig, &sa, NULL) == -1) {
        if (errno == EINVAL) {
            return -1;
        }
        error("sigaction");
    }

    return 0;
}

// this function is non reentrant only for didactic purposes
void sighandler(int sig) {
    printf("sighandler %d\n", sig);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    int sig = SIGKILL, flag = 0;
    char buffer[BUFFER_SIZE];

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
    if (optind == argc - 2) {
        sig = atoi(argv[optind++]);
        flag = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    // set sighandler
    struct sigaction sa;
    mysigaction(sig, NULL, &sa);
    sa.sa_handler = &sighandler;
    mysigaction(sig, &sa, NULL);

    mysiginterrupt(sig, flag);

    myread(STDIN_FILENO, buffer, BUFFER_SIZE);

    exit(EXIT_SUCCESS);
}
