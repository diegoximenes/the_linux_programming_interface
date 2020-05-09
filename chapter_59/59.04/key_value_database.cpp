#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "key_value_database.h"
using namespace std;

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

int mysocket(int domain, int type, int protocol) {
    int sfd = socket(domain, type, protocol);
    if (sfd == -1) {
        error("socket");
    }
    return sfd;
}

void mybind(int socket, const struct sockaddr *address, socklen_t address_len) {
    if (bind(socket, address, address_len) == -1) {
        error("bind");
    }
}

void mywrite(int fd, const char *buf, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t it_written =
            write(fd, buf + total_written, count - total_written);
        if (it_written == -1) {
            error("write");
        }
        total_written += it_written;
    }
}

size_t read_line(int fd, char *buffer, size_t buffer_size) {
    if ((buffer == NULL) || (buffer_size == 0)) {
        errno = EINVAL;
        error("read_line");
    }

    size_t total_read = 0;
    while (1) {
        char c;
        ssize_t num_read = read(fd, &c, 1);
        if (num_read == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                error("read_line");
            }
        } else if ((num_read == 0) || (c == '\n')) {
            break;
        } else if (total_read == buffer_size - 1) {
            fprintf(stderr, "ERROR: read_line buffer size is too small\n");
            exit(EXIT_FAILURE);
        } else {
            buffer[total_read++] = c;
        }
    }
    buffer[total_read] = '\0';

    return total_read;
}

void myclose(int fd) {
    if (close(fd) == -1) {
        error("close");
    }
}
