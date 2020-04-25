#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "seqnum.h"

#define PROGRAM_NAME "client"

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

    // create client message queue name
    char *client_mq_name = mymalloc(NAME_MAX);
    pid_t pid = getpid();
    int client_mq_name_len = format_int_to_string(client_mq_name,
                                                  NAME_MAX,
                                                  "/client_%d_mq",
                                                  pid);

    // create client message queue
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    mqd_t client_mqd = my_mq_open(client_mq_name,
                                  O_CREAT | O_RDONLY,
                                  mode,
                                  NULL);

    // create client message queue buffer
    struct mq_attr client_mq_attr;
    my_mq_getattr(client_mqd, &client_mq_attr);
    char *client_mq_msg = mymalloc(client_mq_attr.mq_msgsize);

    // open server message queue
    mqd_t server_mqd = my_mq_open(SERVER_MQ_NAME, O_WRONLY, 0, NULL);

    // send client queue name to server
    my_mq_send(server_mqd, client_mq_name, client_mq_name_len + 1, 0);

    // receive sequence number from server
    my_mq_receive(client_mqd, client_mq_msg, client_mq_attr.mq_msgsize, NULL);
    int sequence_number = atoi(client_mq_msg);
    printf("pid=%d, receiving sequence_number=%d\n", pid, sequence_number);

    my_mq_close(client_mqd);
    my_mq_close(server_mqd);
    my_mq_unlink(client_mq_name);
    free(client_mq_name);
    free(client_mq_msg);

    exit(EXIT_SUCCESS);
}
