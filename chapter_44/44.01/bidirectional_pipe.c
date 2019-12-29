#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#define PROGRAM_NAME "bidirectional_pipe"

void usage(int status) {
    printf("Usage: %s", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
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

void myclose(int fd) {
    if (close(fd) == -1) {
        error("close");
    }
}

pid_t myfork() {
    pid_t pid = fork();
    if (pid == -1) {
        error("fork");
    }
    return pid;
}

void mypipe(int fildes[2]) {
    if (pipe(fildes) == -1) {
        error("pipe");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "a", long_options, NULL)) != -1) {
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

    // create pipes
    int pipe_to_child_fd[2];
    int pipe_to_parent_fd[2];
    mypipe(pipe_to_child_fd);
    mypipe(pipe_to_parent_fd);

    char buffer[PIPE_BUF];

    pid_t pid = myfork();
    if (pid == 0) {
        // child

        // close unused pipe endpoints
        myclose(pipe_to_child_fd[1]);
        myclose(pipe_to_parent_fd[0]);

        while (1) {
            // read from parent
            ssize_t count_read =
                myread(pipe_to_child_fd[0], buffer, PIPE_BUF);
            if (count_read == 0) {
                break;
            }

            // to upper
            for (ssize_t i = 0; i < count_read; ++i) {
                buffer[i] = toupper(buffer[i]);
            }

            // write to parent
            mywrite(pipe_to_parent_fd[1], buffer, count_read);
        }
    } else {
        // parent

        // close unused pipe endpoints
        myclose(pipe_to_parent_fd[1]);
        myclose(pipe_to_child_fd[0]);

        while (1) {
            // read from stdin
            ssize_t count_read_stdin =
                myread(STDIN_FILENO, buffer, PIPE_BUF);
            if (count_read_stdin == 0) {
                break;
            }

            // write to child
            mywrite(pipe_to_child_fd[1], buffer, count_read_stdin);

            // read from child
            ssize_t count_read_child =
                myread(pipe_to_parent_fd[0], buffer, PIPE_BUF);

            // write to stdout
            mywrite(STDOUT_FILENO, buffer, count_read_child);
        }
    }

    exit(EXIT_SUCCESS);
}
