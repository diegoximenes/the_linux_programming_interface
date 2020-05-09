#include <sys/socket.h>

#define BUFFER_SIZE 4096
#define PORT 50000

void error(const char *msg);

int mysocket(int domain, int type, int protocol);

void mybind(int socket, const struct sockaddr *address, socklen_t address_len);

void mywrite(int fd, const char *buf, size_t count);

size_t read_line(int fd, char *buffer, size_t buffer_size);

void myclose(int fd);
