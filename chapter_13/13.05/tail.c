#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200112L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#define PROGRAM_NAME "tail"

#define BUFFER_SIZE 4096
#define DEFAULT_TAIL_SIZE 10

void usage(int status) {
    printf("Usage: %s [OPTION]... FILE\n", PROGRAM_NAME);
    printf("\n -n NUM output the last NUM lines, instead of the last 10\n");
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

off_t mylseek(int fildes, off_t offset, int whence) {
    off_t ret = lseek(fildes, offset, whence);
    if (ret == -1) {
        error("lseek");
    }
    return ret;
}

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if ((size > 0) && (p == NULL)) {
        error("malloc");
    }
    return p;
}

void myposix_fadvise(int fd, off_t offset, off_t len, int advice) {
    int ret = posix_fadvise(fd, offset, len, advice);
    if (ret != 0) {
        errno = ret;
        error("posix_fadvise");
    }
}

off_t get_tail_offset(int fd, unsigned tail_size, char *buffer) {
    off_t curr_offset = lseek(fd, 0, SEEK_END);

    unsigned cnt_lines = 0;
    while (1) {
        if (curr_offset == 0) {
            break;
        }

        off_t last_offset = curr_offset;
        off_t next_offset = curr_offset - BUFFER_SIZE;
        if (next_offset < 0) {
            next_offset = 0;
        }

        lseek(fd, next_offset, SEEK_SET);
        ssize_t cnt_read = myread(fd, buffer, curr_offset - next_offset);

        for (ssize_t i = cnt_read - 1; i >= 0; --i) {
            if (buffer[i] == '\n') {
                cnt_lines++;
                if (cnt_lines == tail_size + 1) {
                    myposix_fadvise(fd,
                                    curr_offset,
                                    last_offset - curr_offset,
                                    POSIX_FADV_WILLNEED);
                    return curr_offset;
                }
            }
            curr_offset--;
        }

        myposix_fadvise(fd,
                        curr_offset,
                        last_offset - curr_offset,
                        POSIX_FADV_WILLNEED);
    }

    return 0;
}

void write_to_stdout(int fd, off_t offset, char *buffer) {
    lseek(fd, offset, SEEK_SET);
    while (1) {
        ssize_t cnt_read = myread(fd, buffer, BUFFER_SIZE);
        if (cnt_read == 0) {
            break;
        }
        mywrite(STDOUT_FILENO, buffer, cnt_read);
    }
}

void tail(const char *file_path, unsigned tail_size) {
    char *buffer = (char *) mymalloc(BUFFER_SIZE);

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int flags = O_RDONLY;
    int fd = myopen(file_path, mode, flags);

    off_t tail_offset = get_tail_offset(fd, tail_size, buffer);
    write_to_stdout(fd, tail_offset, buffer);

    myclose(fd);
    free(buffer);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *file_path = NULL;
    unsigned tail_size = DEFAULT_TAIL_SIZE;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "n:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'n':
                tail_size = atoi(optarg);
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

    tail(file_path, tail_size);

    exit(EXIT_SUCCESS);
}
