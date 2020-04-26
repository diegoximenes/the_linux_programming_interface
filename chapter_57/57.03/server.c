#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/un.h>
#include "seqnum.h"

#define PROGRAM_NAME "server"

char buffer[BUFFER_SIZE];

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}
int format_int_to_string(char *str,
                         size_t str_len,
                         const char *format,
                         int number) {
    int ret = snprintf(str, str_len, format, number);
    if (ret < 0) {
        fprintf(stderr, "ERROR: snprintf\n");
        exit(EXIT_FAILURE);
    }
    return ret;
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

    myremove(SOCKET_PATH);

    // set server address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    mybind(sfd, (struct sockaddr *) &addr, sizeof(addr));
    mylisten(sfd, 5);

    for (int seq_num = 0; ; seq_num++) {
        int cfd = myaccept(sfd, NULL, NULL);

        int seq_num_str_len = format_int_to_string(buffer,
                                                   BUFFER_SIZE,
                                                   "%d",
                                                   seq_num);
        printf("sending seq_num %d\n", seq_num);
        mywrite(cfd, buffer, seq_num_str_len + 1);

        myclose(cfd);
    }

    exit(EXIT_SUCCESS);
}
