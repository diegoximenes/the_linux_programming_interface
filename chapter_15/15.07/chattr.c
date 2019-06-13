#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define PROGRAM_NAME "chattr"

const char mode_chars[] = "acDijAdtsSTu";

void usage(int status) {
    printf("Usage: %s [-+=%s] FILE_PATH\n", PROGRAM_NAME, mode_chars);
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

int valid_mode(const char *mode) {
    int len = strlen(mode);

    if (len <= 1) {
        return 0;
    }

    if ((mode[0] != '+') && (mode[0] != '-')) {
        return 0;
    }

    // check if all characters in mode are valid
    int len_mode_chars = strlen(mode_chars);
    for (int i = 1; i < len; ++i) {
        int in_mode_chars = 0;
        for (int j = 0; j < len_mode_chars; ++j) {
            if (mode_chars[j] == mode_chars[i]) {
                in_mode_chars = 1;
                break;
            }
        }
        if (!in_mode_chars) {
            return 0;
        }
    }

    return 1;
}

int get_attr_from_char(char c) {
    switch (c) {
        case 'a':
            return FS_APPEND_FL;
        case 'c':
            return FS_COMPR_FL;
        case 'D':
            return FS_DIRSYNC_FL;
        case 'i':
            return FS_IMMUTABLE_FL;
        case 'j':
            return FS_JOURNAL_DATA_FL;
        case 'A':
            return FS_NOATIME_FL;
        case 'd':
            return FS_NODUMP_FL;
        case 't':
            return FS_NOTAIL_FL;
        case 's':
            return FS_SECRM_FL;
        case 'S':
            return FS_SYNC_FL;
        case 'T':
            return FS_TOPDIR_FL;
        case 'u':
            return FS_UNRM_FL;
        default:
            return 0;
    }
}

void chattr(const char *mode, const char *file_path) {
    int flags = O_WRONLY;
    int fd = myopen(file_path, flags);

    int attr = 0;
    int len  = strlen(mode);
    for (int i = 1; i < len; ++i) {
        attr |= get_attr_from_char(mode[i]);
    }

    if (mode[0] == '-') {
        int curr_attr;
        if (ioctl(fd, FS_IOC_GETFLAGS, &curr_attr) == -1) {
            error("ioctl");
        }
        attr = curr_attr & (~attr);
    }

    if (ioctl(fd, FS_IOC_SETFLAGS, &attr) == -1) {
        error("ioctl");
    }

    myclose(fd);
}

int main(int argc, char *argv[]) {
    char *mode = NULL;
    char *file_path = NULL;

    if (argc == 3) {
        mode = argv[1];
        file_path = argv[2];
    } else {
        usage(EXIT_FAILURE);
    }

    if (!valid_mode(mode)) {
        usage(EXIT_FAILURE);
    } else {
        chattr(mode, file_path);
    }

    exit(EXIT_SUCCESS);
}
