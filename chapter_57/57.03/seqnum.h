#include <sys/socket.h>

#define SOCKET_PATH "/tmp/seqnum_socket"
#define BUFFER_SIZE 4096

void error(const char *msg);

int mysocket(int domain, int type, int protocol);

void mybind(int socket, const struct sockaddr *address, socklen_t address_len);

void mylisten(int socket, int backlog);

int myaccept(int socket, struct sockaddr *address, socklen_t *address_len);

void myconnect(int socket,
               const struct sockaddr *address,
               socklen_t address_len);

void myclose(int fd);

void mywrite(int fd, const char *buf, size_t count);

ssize_t myread(int fd, char *buf, size_t count);

void myremove(const char *pathname);
