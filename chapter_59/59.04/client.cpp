#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "key_value_database.h"
using namespace std;

#define PROGRAM_NAME "client"

char msg[BUFFER_SIZE];

void usage(int status) {
    printf("Usage: %s SERVER_ADDRESS OPERATION KEY VALUE\n", PROGRAM_NAME);
    printf("OPERATION must be add, delete, get\n");
    printf("If OPERATION is add then VALUE must be informed\n");
    exit(status);
}

void my_inet_pton(int af, const char *src, void *dst) {
    int ret = inet_pton(af, src, dst);
    if (ret == 0) {
        fprintf(stderr,
                "ERROR: inet_pton %s doesn't contain a valid address\n",
                src);
        exit(EXIT_FAILURE);
    }
}

void myconnect(int socket,
               const struct sockaddr *address,
               socklen_t address_len) {
    if (connect(socket, address, address_len) == -1) {
        error("connect");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *server_address_buffer;
    char *operation, *key, *value;

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
    if (argc < 4) {
        usage(EXIT_FAILURE);
    } else {
        server_address_buffer = argv[optind++];
        operation = argv[optind++];
        key = argv[optind++];

        if ((strcmp(operation, "add") != 0) &&
                (strcmp(operation, "delete") != 0) &&
                (strcmp(operation, "get") != 0)) {
            usage(EXIT_FAILURE);
        }

        if (strcmp(operation, "add") == 0) {
            if (argc != 5) {
                usage(EXIT_FAILURE);
            }
            value = argv[optind++];
        } else if (argc != 4) {
            usage(EXIT_FAILURE);
        }
    }

    int sfd = mysocket(AF_INET6, SOCK_STREAM, 0);

    // set server address
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(PORT);
    my_inet_pton(AF_INET6, server_address_buffer, &server_addr.sin6_addr);

    myconnect(sfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    // build message
    if (strcmp(operation, "add") == 0) {
        sprintf(msg, "%s,%s,%s\n", operation, key, value);
    } else {
        sprintf(msg, "%s,%s\n", operation, key);
    }

    // send message and receive response
    mywrite(sfd, msg, strlen(msg));
    read_line(sfd, msg, BUFFER_SIZE);
    printf("%s\n", msg);

    myclose(sfd);

    exit(EXIT_SUCCESS);
}
