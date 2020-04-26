#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "seqnum.h"

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

void myconnect(int socket,
               const struct sockaddr *address,
               socklen_t address_len) {
    if (connect(socket, address, address_len) == -1) {
        error("connect");
    }
}

void myclose(int fd) {
    if (close(fd) == -1) {
        error("close");
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

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
}

void myremove(const char *pathname) {
    if ((remove(pathname) == -1) && (errno != ENOENT)) {
        error("remove");
    }
}
