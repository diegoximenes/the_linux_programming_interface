#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#define PROGRAM_NAME "cp_mmap"

void usage(int status) {
    printf("Usage: %s SOURCE DEST", PROGRAM_NAME);
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

void mymsync(void *addr, size_t len, int flags) {
    if (msync(addr, len, flags) == -1) {
        error("msync");
    }
}

void myunmap(void *addr, size_t len) {
    if (munmap(addr, len) == -1) {
        error("munmap");
    }
}

void myfstat(int fildes, struct stat *buf) {
    if (fstat(fildes, buf) == -1) {
        error("fstat");
    }
}

void myftruncate(int fildes, off_t length) {
    if (ftruncate(fildes, length) == -1) {
        error("ftruncate");
    }
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *source_path, *dest_path;

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
    if (optind == argc - 2) {
        source_path = argv[optind++];
        dest_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    int source_fd = myopen(source_path, O_RDONLY, 0);
    struct stat source_stat;
    myfstat(source_fd, &source_stat);
    char *source_mmap = mymmap(NULL,
                               source_stat.st_size,
                               PROT_READ,
                               MAP_PRIVATE,
                               source_fd,
                               0);

    int flags = O_RDWR | O_CREAT | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int dest_fd = myopen(dest_path, flags, mode);
    myftruncate(dest_fd, source_stat.st_size);
    char *dest_mmap = mymmap(NULL,
                             source_stat.st_size,
                             PROT_WRITE,
                             MAP_SHARED,
                             dest_fd,
                             0);

    memcpy(dest_mmap, source_mmap, source_stat.st_size);

    msync(dest_mmap, source_stat.st_size, MS_SYNC);

    myunmap(source_mmap, source_stat.st_size);
    myunmap(dest_mmap, source_stat.st_size);

    myclose(source_fd);
    myclose(dest_fd);

    exit(EXIT_SUCCESS);
}
