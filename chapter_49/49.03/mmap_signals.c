#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define PROGRAM_NAME "mmap_signals"

void usage(int status) {
    printf("Usage: %s PATH OFFSET", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

int myopen(const char *pathname, int flags) {
    int fd = open(pathname, flags);
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

void *mymmap(void *addr,
             size_t len,
             int prot,
             int flags,
             int fildes,
             off_t off) {
    void *mmap_addr = mmap(addr, len, prot, flags, fildes, off);
    if (mmap_addr == MAP_FAILED) {
        error("mmap");
    }
    return mmap_addr;
}

void mymunmap(void *addr, size_t len) {
    if (munmap(addr, len) == -1) {
        error("munmap");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *path;
    int offset, mmap_size;

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
    if (argc != 4) {
        usage(EXIT_FAILURE);
    } else {
        path = argv[optind++];
        mmap_size = atoi(argv[optind++]);
        offset = atoi(argv[optind++]);
    }

    int fd = myopen(path, O_RDONLY);
    char *mmap_addr = mymmap(NULL,
                             mmap_size,
                             PROT_READ,
                             MAP_SHARED,
                             fd,
                             0);

    printf("mmap_addr[%d]=%c\n", offset, mmap_addr[offset]);

    mymunmap(mmap_addr, mmap_size);
    myclose(fd);

    exit(EXIT_SUCCESS);
}
