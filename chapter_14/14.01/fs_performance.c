#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PROGRAM_NAME "fs_performance"
#define N_RAND 20

void usage(int status) {
    printf("Usage: %s NUM_FILES DIR DEL_SORTED\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void *mymalloc(size_t size) {
    void *p = malloc(size);
    if ((size > 0) && (p == NULL)) {
        error("malloc");
    }
    return p;
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

void myremove(const char *pathname) {
    if (remove(pathname) == -1) {
        error("remove");
    }
}

void rand_str(char *str, int n) {
    for (int i = 0; i < n; ++i) {
        str[i] = '0' + rand() % 10;
    }
}

void create_files(const char *dir, unsigned num_files, char **file_path) {
    int dir_len = strlen(dir);
    int path_len = dir_len + 1 + N_RAND;
    char *path = (char *) mymalloc(path_len + 1);
    strcpy(path, dir);
    path[dir_len] = '/';
    path[path_len] = '\0';

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int flags = O_WRONLY | O_CREAT | O_APPEND;

    srand(time(NULL));
    for (unsigned i = 0; i < num_files; ++i) {
        rand_str(path + dir_len + 1, N_RAND);
        int fd = myopen(path, flags, mode);
        char c = 'a';
        mywrite(fd, &c, 1);
        myclose(fd);

        file_path[i] = (char *) mymalloc(path_len + 1);
        strcpy(file_path[i], path);
    }

    free(path);
}

int cmp(const void* p1, const void* p2) {
    const char **char_p1 = (const char **) p1;
    const char **char_p2 = (const char **) p2;
    return strcmp(*char_p1, *char_p2);
}

void del_files(char **file_path, unsigned num_files, int del_sorted) {
    if (del_sorted) {
        qsort(file_path, num_files, sizeof(char *), cmp);
    }

    time_t start_time = time(NULL);
    for (unsigned i = 0; i < num_files; ++i) {
        myremove(file_path[i]);
    }
    time_t end_time = time(NULL);
    printf("time to delete: %ld sec\n", end_time - start_time);
}

void create_and_delete(const char *dir, unsigned num_files, int del_sorted) {
    char **file_path = (char **) mymalloc(sizeof(char *) * num_files);

    create_files(dir, num_files, file_path);
    del_files(file_path, num_files, del_sorted);

    for (unsigned i = 0; i < num_files; ++i) {
        free(file_path[i]);
    }
    free(file_path);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    // program variables
    char *dir = NULL;
    unsigned num_files = 0, del_sorted = 0;

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
        num_files = atoi(argv[optind++]);
        dir = argv[optind++];
        del_sorted = atoi(argv[optind++]);
    } else {
       usage(EXIT_FAILURE);
    }

    create_and_delete(dir, num_files, del_sorted);

    exit(EXIT_SUCCESS);
}
