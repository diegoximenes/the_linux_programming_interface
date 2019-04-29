#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "dup_fcntl"

#define BUFFER_SIZE 4096

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

int myopen(const char *pathname, int flags, mode_t mode) {
    int fd = open(pathname, flags, mode);
    if (fd == -1) {
        error("open");
    }
    return fd;
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

int mydup(int oldfd) {
    return fcntl(oldfd, F_DUPFD, 1);
}

int mydup2(int oldfd, int newfd) {
    if (oldfd == newfd) {
        int flags = fcntl(newfd, F_GETFL);
        if (flags == -1) {
            return -1;
        }
        return newfd;
    }
    close(newfd);
    return fcntl(oldfd, F_DUPFD, newfd);
}

int main(int argc, char *argv[]) {
    // getopt vars
    int opt;
    // program vars
    int fd, fd_dup, fd_dup2, fd_dup3, flags;
    mode_t mode;

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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    flags = O_WRONLY | O_CREAT | O_TRUNC;
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    fd = myopen("./test.txt", flags, mode);
    fd_dup = mydup(fd);
    fd_dup2 = mydup2(fd, 10);
    printf("fd=%d, fd_dup=%d, fd_dup2=%d\n", fd, fd_dup, fd_dup2);

    mywrite(fd, "hello", 5);
    mywrite(fd_dup, "world", 5);
    mywrite(fd_dup2, "again", 5);

    myclose(fd);
    myclose(fd_dup2);

    fd_dup3 = mydup2(100, 100);
    if (fd_dup3 == -1) {
        error("mydup2");
    }

    exit(EXIT_SUCCESS);
}
