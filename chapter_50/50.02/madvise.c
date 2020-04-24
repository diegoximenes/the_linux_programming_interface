#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <sys/mman.h>

#define PROGRAM_NAME "madvise"

void usage(int status) {
    printf("Usage: %s PATH OFFSET\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
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

void mymadvise(void *addr, size_t length, int advice) {
    if (madvise(addr, length, advice) == -1) {
        error("madvise");
    }
}

void print_mem(char *addr, int len) {
    for (size_t i = 0; i < len; ++i) {
        printf("addr[%ld]=%c\n", i, addr[i]);
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
    if (argc != 1) {
        usage(EXIT_FAILURE);
    }

    size_t len = 10;
    char *addr = mymmap(NULL,
                        len,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1,
                        0);

    memset(addr, 'a', len);

    printf("before madvise:\n");
    print_mem(addr, len);

    mymadvise(addr, len, MADV_DONTNEED);

    printf("after madvise:\n");
    print_mem(addr, len);

    memset(addr, 'b', len);
    printf("after memset b:\n");
    print_mem(addr, len);

    mymunmap(addr, len);

    exit(EXIT_SUCCESS);
}
