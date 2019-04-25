#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "cp_holes"

#define BUFFER_SIZE 1000

void usage(int status) {
    printf("Usage: %s SOURCE DEST DEST_NAIVE\n", PROGRAM_NAME);
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
    int opt;
    char *source_path = NULL, *dest_path = NULL, *dest_naive_path = NULL;
    char buffer[BUFFER_SIZE];
    ssize_t cnt_read;
    int source_fd, dest_fd, dest_naive_fd;
    int flags;
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
    if (optind == argc - 3) {
        source_path = argv[optind++];
        dest_path = argv[optind++];
        dest_naive_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    // open source file
    source_fd = myopen(source_path, O_RDONLY, 0);

    // open destination files
    flags = O_WRONLY | O_CREAT | O_TRUNC;
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    dest_fd = myopen(dest_path, flags, mode);
    dest_naive_fd = myopen(dest_naive_path, flags, mode);

    while ((cnt_read = myread(source_fd, buffer, BUFFER_SIZE)) > 0) {
        // naive copy
        mywrite(dest_naive_fd, buffer, cnt_read);

        // copy with holes
        ssize_t l = 0;
        while (l < cnt_read) {
            ssize_t r;
            if (buffer[l] != '\0') {
                for (r = l + 1; (r < cnt_read) && (buffer[r] != '\0'); r++);
                mywrite(dest_fd, buffer + l, r - l);
            } else {
                for (r = l + 1; (r < cnt_read) && (buffer[r] == '\0'); r++);
                lseek(dest_fd, r - l, SEEK_CUR);
            }
            l = r;
        }
    }

    myclose(source_fd);
    myclose(dest_fd);
    myclose(dest_naive_fd);

    exit(EXIT_SUCCESS);
}
