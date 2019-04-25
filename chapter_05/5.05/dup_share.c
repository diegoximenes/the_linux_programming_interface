#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "dup_share"

#define BUFFER_SIZE 4096

void usage(int status) {
    printf("Usage: %s FILE\n", PROGRAM_NAME);
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

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *file_path = NULL;
    // program variables
    int fd1, fd2, flags;
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
    if (optind == argc - 1) {
        file_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    flags = O_WRONLY | O_CREAT;

    fd1 = myopen(file_path, flags, mode);
    fd2 = dup(fd1);

    // if fd1 and fd2 shares offset then the output will be "Hello world"
    mywrite(fd1, "hello", 5);
    mywrite(fd2, " world", 6);
    lseek(fd2, 0, SEEK_SET);
    mywrite(fd1, "H", 1);

    myclose(fd1);
    myclose(fd2);

    exit(EXIT_SUCCESS);
}
