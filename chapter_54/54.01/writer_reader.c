#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "writer_reader.h"


void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void my_sem_init(sem_t *sem, int pshared, unsigned int value) {
    if (sem_init(sem, pshared, value) == -1) {
        error("sem_init");
    }
}

void my_sem_wait(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        error("sem_wait");
    }
}

void my_sem_post(sem_t *sem) {
    if (sem_post(sem) == -1) {
        error("sem_wait");
    }
}

int my_shm_open(const char *name, int oflag, mode_t mode) {
    int ret = shm_open(name, oflag, mode);
    if (ret == -1) {
        error("shm_open");
    }
    return ret;
}

void myftruncate(int fildes, off_t length) {
    if (ftruncate(fildes, length) == -1) {
        error("ftruncate");
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

void* get_shm(const char *name, int oflag, size_t length, int prot) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int fd = my_shm_open(name, oflag, mode);
    if (oflag & O_CREAT) {
        myftruncate(fd, length);
    }
    return mymmap(NULL, length, prot, MAP_SHARED, fd, 0);
}

sem_t* get_semaphore(const char *name, int oflag) {
    return get_shm(name, oflag, sizeof(sem_t), PROT_READ | PROT_WRITE);
}
