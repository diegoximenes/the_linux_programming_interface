#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "tee"

#define BUFFER_SIZE 4096

void usage(int status) {
    printf("Usage: %s [OPTION]... FILE\n", PROGRAM_NAME);
    printf("\n -a append text to the end of a file if it already exists\n");
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

ssize_t myread(int fd, char *buf, size_t count) {
    ssize_t cnt_read = read(fd, buf, count);
    if (cnt_read == -1) {
        error("read");
    }
    return cnt_read;
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
    int opt, opt_a_flag = 0;
    char *file_path = NULL;
    // program variables
    char buffer[BUFFER_SIZE];
    ssize_t cnt_read;
    int out_fd, flags;
    mode_t mode;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "a", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                opt_a_flag = 1;
                break;
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
    flags = O_WRONLY | O_CREAT | O_APPEND;
    if (!opt_a_flag) {
        flags |= O_TRUNC;
    }
    out_fd = myopen(file_path, flags, mode);

    while ((cnt_read = myread(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        mywrite(STDOUT_FILENO, buffer, cnt_read);
        mywrite(out_fd, buffer, cnt_read);
    }

    myclose(out_fd);

    exit(EXIT_SUCCESS);
}
