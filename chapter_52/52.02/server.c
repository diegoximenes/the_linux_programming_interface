#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "seqnum.h"

#define PROGRAM_NAME "server"

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

    // create server message queue
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    mqd_t server_mqd = my_mq_open(SERVER_MQ_NAME,
                                  O_CREAT | O_RDONLY,
                                  mode,
                                  NULL);

    // create server message queue buffer
    struct mq_attr server_mq_attr;
    my_mq_getattr(server_mqd, &server_mq_attr);
    char *server_mq_msg = mymalloc(server_mq_attr.mq_msgsize);

    int sequence_number = 0;

    while (1) {
        // read client message queue name
        my_mq_receive(server_mqd,
                      server_mq_msg,
                      server_mq_attr.mq_msgsize,
                      NULL);

        // open client message queue
        mqd_t client_mqd = my_mq_open(server_mq_msg, O_WRONLY, 0, NULL);

        // create client message queue buffer
        struct mq_attr client_mq_attr;
        my_mq_getattr(client_mqd, &client_mq_attr);
        char *client_mq_msg = mymalloc(client_mq_attr.mq_msgsize);

        // create sequence number message
        int client_mq_msg_len = format_int_to_string(client_mq_msg,
                                                     client_mq_attr.mq_msgsize,
                                                     "%d",
                                                     sequence_number);

        printf("sending %s through %s\n", client_mq_msg, server_mq_msg);
        my_mq_send(client_mqd, client_mq_msg, client_mq_msg_len + 1, 0);

        free(client_mq_msg);
        my_mq_close(client_mqd);

        sequence_number++;
    }

    my_mq_close(server_mqd);
    my_mq_unlink(SERVER_MQ_NAME);
    free(server_mq_msg);

    exit(EXIT_SUCCESS);
}
