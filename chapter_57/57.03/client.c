#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/un.h>
#include "seqnum.h"

#define PROGRAM_NAME "client"

char buffer[BUFFER_SIZE];

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
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
    if (argc != 1) {
        usage(EXIT_FAILURE);
    }

    int sfd = mysocket(AF_UNIX, SOCK_STREAM, 0);

    // set server address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    myconnect(sfd, (struct sockaddr *) &addr, sizeof(addr));

    int total_read = 0;
    while (1) {
        int num_read = myread(sfd,
                              buffer + total_read,
                              BUFFER_SIZE - total_read);
        total_read += num_read;
        if (num_read == 0) {
            break;
        }
    }

    int seq_num = atoi(buffer);
    printf("seq_num=%d\n", seq_num);

    exit(EXIT_SUCCESS);
}
