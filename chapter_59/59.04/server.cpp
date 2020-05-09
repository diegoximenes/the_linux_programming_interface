#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unordered_map>
#include "key_value_database.h"
using namespace std;

#define PROGRAM_NAME "server"

char msg[BUFFER_SIZE];
char operation[BUFFER_SIZE], key[BUFFER_SIZE], value[BUFFER_SIZE];

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

void mylisten(int socket, int backlog) {
    if (listen(socket, backlog) == -1) {
        error("listen");
    }
}

int myaccept(int socket, struct sockaddr *address, socklen_t *address_len) {
    int sfd = accept(socket, address, address_len);
    if (sfd == -1) {
        error("accept");
    }
    return sfd;
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

    int sfd = mysocket(AF_INET6, SOCK_STREAM, 0);

    struct sockaddr_in6 client_addr;
    // server address
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(PORT);

    socklen_t addr_len = sizeof(struct sockaddr_in6);

    mybind(sfd, (struct sockaddr *) &server_addr, addr_len);
    mylisten(sfd, 5);

    unordered_map<string, string> hash_table;
    while (1) {
        int cfd = myaccept(sfd, (struct sockaddr *) &client_addr, &addr_len);

        read_line(cfd, msg, BUFFER_SIZE);

        // parse msg
        unsigned curr_idx = 0;
        const char *delim = ",";
        char *token = strtok(msg, delim);
        while (token != NULL) {
            if (curr_idx == 0) {
                strncpy(operation, token, BUFFER_SIZE);
            } else if (curr_idx == 1) {
                strncpy(key, token, BUFFER_SIZE);
            } else {
                strncpy(value, token, BUFFER_SIZE);
            }
            token = strtok(NULL, delim);
            curr_idx++;
        }

        // update hash_table
        if (strcmp(operation, "add") == 0) {
            hash_table[key] = value;
            sprintf(msg, "ok\n");
        } else if (strcmp(operation, "get") == 0) {
            if (hash_table.find(key) == hash_table.end()) {
                sprintf(msg, "error,not_found\n");
            } else {
                sprintf(msg, "ok,%s\n", hash_table[key].c_str());
            }
        } else if (strcmp(operation, "delete") == 0) {
            if (hash_table.find(key) == hash_table.end()) {
                sprintf(msg, "error,not_found\n");
            } else {
                hash_table.erase(key);
                sprintf(msg, "ok\n");
            }
        } else {
            sprintf(msg, "error,invalid_operation\n");
        }

        // send response
        mywrite(cfd, msg, strlen(msg));

        myclose(cfd);
    }

    myclose(sfd);

    exit(EXIT_SUCCESS);
}
