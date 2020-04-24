#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/mman.h>

#define PROGRAM_NAME "rlimit_memlock"

void usage(int status) {
    printf("Usage: %s PATH OFFSET\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mygetrlimit(int resource, struct rlimit *rlp) {
    if (getrlimit(resource, rlp) == -1) {
        error("getrlimit");
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

long mysysconf(int name) {
    errno = 0;
    long ret = sysconf(name);
    if (errno != 0) {
        error("sysconf");
    }
    return ret;
}

void mymlock(const void *addr, size_t len) {
    if (mlock(addr, len) == -1) {
        error("mlock");
    }
}

void test_mlock(size_t len) {
    char *addr = mymmap(NULL,
                        len,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1,
                        0);
    mymlock(addr, len);
    mymunmap(addr, len);
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

    struct rlimit rlp;
    mygetrlimit(RLIMIT_MEMLOCK, &rlp);
    long page_size = mysysconf(_SC_PAGESIZE);

    if (rlp.rlim_cur < page_size) {
        printf("rlp.rlim_cur=%ld, page_size=%ld, not able to mlock\n",
               rlp.rlim_cur, page_size);
    } else {
        printf("test_mlock(1):\n");
        test_mlock(1);
    }

    size_t pages_to_allocate_exceed =
        rlp.rlim_cur / page_size + rlp.rlim_cur % page_size + 1;
    size_t bytes_to_allocate_exceed = pages_to_allocate_exceed * page_size;
    printf("\ntest_mlock(%ld):\n", bytes_to_allocate_exceed);
    test_mlock(bytes_to_allocate_exceed);

    exit(EXIT_SUCCESS);
}
