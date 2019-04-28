#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>

#define PROGRAM_NAME "scatter"

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

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if (size > 0 && p == NULL) {
        error("malloc");
    }
    return p;
}

ssize_t mywritev(int fd, const struct iovec *iov, int iovcnt) {
    int i, total_written = 0, written;

    for (i = 0; i < iovcnt; i++) {
        written = write(fd, iov[i].iov_base, iov[i].iov_len);
        total_written += written;
        if (written == -1) {
            return -1;
        } else if ((size_t) written < iov[i].iov_len) {
            return total_written;
        }
    }

    return total_written;
}

ssize_t myreadv(int fd, const struct iovec *iov, int iovcnt) {
    int i, total_read = 0, cnt_read;

    for (i = 0; i < iovcnt; i++) {
        cnt_read = read(fd, iov[i].iov_base, iov[i].iov_len);
        total_read += cnt_read;
        if (cnt_read == -1) {
            return -1;
        } else if ((size_t) cnt_read < iov[i].iov_len) {
            return total_read;
        }
    }

    return total_read;
}

int main(int argc, char *argv[]) {
    // opt vars
    int opt;
    // program vars
    char c;
    int flags, fd_write, fd_read, i;
    mode_t mode;
    struct iovec iov[3];

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

    iov[0].iov_len = 5;
    iov[1].iov_len = 7;
    iov[2].iov_len = 10;
    for (i = 0, c = 'a'; i < 3; ++i, ++c) {
        iov[i].iov_base = mymalloc(iov[i].iov_len);
        memset(iov[i].iov_base, c, iov[i].iov_len);
    }

    flags = O_WRONLY | O_CREAT | O_TRUNC;
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    fd_write = myopen("./write.txt", flags, mode);
    mywritev(fd_write, iov, 3);

    flags = O_RDONLY;
    fd_read = myopen("./read.txt", flags, mode);
    readv(fd_read, iov, 3);
    for (i = 0; i < 3; ++i) {
        printf("i=%d, iov=%s\n", i, (char *) iov[i].iov_base);
    }

    for (i = 0; i < 3; ++i) {
        free(iov[i].iov_base);
    }

    myclose(fd_write);
    myclose(fd_read);

    exit(EXIT_SUCCESS);
}
