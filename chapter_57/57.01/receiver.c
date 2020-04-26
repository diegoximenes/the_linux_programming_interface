#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "sender_receiver.h"

#define PROGRAM_NAME "receiver"

#define BUFFER_SIZE 4096

char buffer[BUFFER_SIZE];

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

void myusleep(useconds_t usec) {
    if (usleep(usec) == -1) {
        error("usleep");
    }
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

    int sfd = mysocket(AF_UNIX, SOCK_DGRAM, 0);

    // set receiver message
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, RECEIVER_SOCKET_PATH, sizeof(addr.sun_path));
    socklen_t addr_len = sizeof(addr);

    mybind(sfd, (struct sockaddr *) &addr, sizeof(addr));

    for (int i = 0; ; i++) {
        int msg_len = myrecvfrom(sfd,
                                 buffer,
                                 BUFFER_SIZE,
                                 0,
                                 (struct sockaddr *) &addr,
                                 &addr_len);
        printf("received message %d: %.*s\n", i, msg_len, buffer);
        usleep(50000);
    }

    exit(EXIT_SUCCESS);
}
