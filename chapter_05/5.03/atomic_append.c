#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "atomic_append"

void usage(int status) {
    printf("Usage: %s [OPTION]... FILE NUM_BYTES\n", PROGRAM_NAME);
    printf("\n -x don't use O_APPEND\n");
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
    int opt, opt_x_flag = 0;
    char *file_path = NULL;
    // program variables
    int out_fd, flags, num_bytes = 0;
    mode_t mode;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "x", long_options, NULL)) != -1) {
        switch (opt) {
            case 'x':
                opt_x_flag = 1;
                break;
            case GETOPT_HELP_CHAR:
                usage(EXIT_SUCCESS);
                break;
            default:
                usage(EXIT_FAILURE);
                break;
        }
    }
    if (optind == argc - 2) {
        file_path = argv[optind++];
        num_bytes = atoi(argv[optind++]);
    } else {
        usage(EXIT_FAILURE);
    }

    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    flags = O_WRONLY | O_CREAT;
    if (!opt_x_flag) {
        flags |= O_APPEND;
    }
    out_fd = myopen(file_path, flags, mode);

    for (; num_bytes > 0; --num_bytes) {
        char buf = 'a';
        mywrite(out_fd, &buf, 1);
    }

    myclose(out_fd);

    exit(EXIT_SUCCESS);
}
