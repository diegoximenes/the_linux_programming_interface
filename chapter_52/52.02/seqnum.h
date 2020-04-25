#include <mqueue.h>

#define SERVER_MQ_NAME "/server_mq"

void error(const char *msg);

void *mymalloc(size_t size);

mqd_t my_mq_open(const char *name,
                 int oflag,
                 mode_t mode,
                 struct mq_attr *attr);

void my_mq_close(mqd_t mqdes);

void my_mq_unlink(const char *name);

ssize_t my_mq_receive(mqd_t mqdes,
                      char *msg_ptr,
                      size_t msg_len,
                      unsigned int *msg_prio);

void my_mq_send(mqd_t mqdes,
                const char *msg_ptr,
                size_t msg_len,
                unsigned int msg_prio);

void my_mq_getattr(mqd_t mqdes, struct mq_attr *attr);

int format_int_to_string(char *str,
                          size_t str_len,
                          const char *format,
                          int number);
