#include <semaphore.h>

#define BUFFER_SIZE 4096

#define SEMAPHORE_WRITER_NAME "writer"
#define SEMAPHORE_READER_NAME "reader"
#define BUFFER_NAME "buffer"
#define BUFFER_LEN_NAME "buffer_len"

void error(const char *msg);

void my_sem_init(sem_t *sem, int pshared, unsigned int value);
void my_sem_wait(sem_t *sem);
void my_sem_post(sem_t *sem);

int my_shm_open(const char *name, int oflag, mode_t mode);

void myftruncate(int fildes, off_t length);

void *mymmap(void *addr,
             size_t len,
             int prot,
             int flags,
             int fildes,
             off_t off);

sem_t* get_semaphore(const char *name, int oflag);
void* get_shm(const char *name, int oflag, size_t length, int prot);
