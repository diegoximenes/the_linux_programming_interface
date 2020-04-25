#include <stdio.h>
#include <stdlib.h>
#include "seqnum.h"

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if ((size > 0) && (p == NULL)) {
        error("malloc");
    }
    return p;
}

mqd_t my_mq_open(const char *name,
                 int oflag,
                 mode_t mode,
                 struct mq_attr *attr) {
    mqd_t ret = mq_open(name, oflag, mode, attr);
    if (ret == -1) {
        error("mq_open");
    }
    return ret;
}

void my_mq_close(mqd_t mqdes) {
    if (mq_close(mqdes) == -1) {
        error("mq_close");
    }
}

void my_mq_unlink(const char *name) {
    if (mq_unlink(name) == -1) {
        error("mq_unlink");
    }
}

ssize_t my_mq_receive(mqd_t mqdes,
                      char *msg_ptr,
                      size_t msg_len,
                      unsigned int *msg_prio) {
    ssize_t ret = mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
    if (ret == -1) {
        error("mq_receive");
    }
    return ret;
}

void my_mq_send(mqd_t mqdes,
                const char *msg_ptr,
                size_t msg_len,
                unsigned int msg_prio) {
    if (mq_send(mqdes, msg_ptr, msg_len, msg_prio) == -1) {
        error("mq_send");
    }
}

void my_mq_getattr(mqd_t mqdes, struct mq_attr *attr) {
    if (mq_getattr(mqdes, attr) == -1) {
        error("mq_getattr");
    }
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
