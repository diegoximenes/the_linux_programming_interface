#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

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

void mysendto(int socket,
              const void *message,
              size_t length,
              int flags,
              const struct sockaddr *dest_addr,
              socklen_t dest_len) {
    ssize_t bytes_sent = sendto(socket,
                                message,
                                length,
                                flags,
                                dest_addr,
                                dest_len);
    if (bytes_sent == -1) {
        error("sendto");
    } else if (bytes_sent < length) {
        fprintf(stderr,
                "ERROR: sendto supposed to send %ld bytes but sent %ld",
                length,
                bytes_sent);
        exit(EXIT_FAILURE);
    }
}

ssize_t myrecvfrom(int socket,
                   void *buffer,
                   size_t length,
                   int flags,
                   struct sockaddr *address,
                   socklen_t *address_len) {
    ssize_t msg_len = recvfrom(socket,
                               buffer,
                               length,
                               flags,
                               address,
                               address_len);
    if (msg_len == -1) {
        error("recvfrom");
    }
    return msg_len;
}
