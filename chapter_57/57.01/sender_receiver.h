#include <sys/socket.h>

#define RECEIVER_SOCKET_PATH "/tmp/sender_receiver_socket"

void error(const char *msg);

int mysocket(int domain, int type, int protocol);

void mybind(int socket, const struct sockaddr *address, socklen_t address_len);

void mysendto(int socket,
              const void *message,
              size_t length,
              int flags,
              const struct sockaddr *dest_addr,
              socklen_t dest_len);

ssize_t myrecvfrom(int socket,
                   void *buffer,
                   size_t length,
                   int flags,
                   struct sockaddr *address,
                   socklen_t *address_len);
