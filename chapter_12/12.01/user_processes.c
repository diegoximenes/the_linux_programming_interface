#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>

#define PROGRAM_NAME "user_processes"

void usage(int status) {
    printf("Usage: %s USER_NAME\n", PROGRAM_NAME);
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

int valid_integer(const char *s) {
    char *endptr = NULL;
    errno = 0;
    strtol(s, &endptr, 10);
    if ((s == endptr) || (errno != 0)) {
        return 0;
    }
    return 1;
}

DIR *myopendir(const char *path, int exit_on_error) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        if ((exit_on_error == 1) || (errno != ENOENT)) {
            error("opendir");
        }
    }
    return dir;
}

struct dirent *myreaddir(DIR *dir, int exit_on_error) {
    errno = 0;
    struct dirent *dir_entry = readdir(dir);
    if (dir_entry == NULL) {
        if ((errno != 0) && (exit_on_error == 1)) {
            error("readdir");
        }
    }
    return dir_entry;
}

FILE *myfopen(const char *pathname, const char *mode, int exit_on_error) {
    FILE *f = fopen(pathname, mode);
    if ((f == NULL) && ((exit_on_error == 1) || (errno != ENOENT))) {
        error("fopen");
    }
    return f;
}

void myfclose(FILE *stream) {
    if (fclose(stream) != 0) {
        error("fclose");
    }
}

ssize_t mygetline(char **line, FILE *f) {
    size_t n;
    errno = 0;
    ssize_t read = getline(line, &n, f);
    if (read == -1) {
        if (errno != 0) {
            error("getline");
        }
    }
    return read;
}

char *split(char *str, const char *delim, unsigned token_idx) {
    if (str == NULL) {
        return NULL;
    }
    unsigned curr_idx = 0;
    char *token = strtok(str, delim);
    while ((curr_idx < token_idx) && (token != NULL)) {
        token = strtok(NULL, delim);
        curr_idx++;
    }
    return token;
}

struct passwd *mygetpwuid(uid_t uid) {
    errno = 0;
    struct passwd *p = getpwuid(uid);
    if ((p == NULL) && (errno != 0)) {
        error("getpwuid");
    }
    return p;
}

char *get_user_name(uid_t uid) {
    struct passwd *p = mygetpwuid(uid);
    if (p != NULL) {
        return p->pw_name;
    } else {
        return NULL;
    }
}

void get_status(const char *pid, char **status_user_name, char **command_name) {
    *status_user_name = NULL;
    *command_name = NULL;

    // build status file path
    char *f_path = (char *) mymalloc(6 + strlen(pid) + 7 + 1);
    strcpy(f_path, "/proc/");
    strcpy(f_path + 6, pid);
    strcpy(f_path + strlen(f_path), "/status");

    FILE *f = myfopen(f_path, "r", 0);
    free(f_path);
    // process related to pid can be killed between readdir and this fopen
    if (f == NULL) {
        return;
    }

    // iterate through lines of status file
    char *line = (char *) mymalloc(4096);
    while (1) {
        ssize_t read = mygetline(&line, f);
        if (read == -1) {
            // EOF
            break;
        }

        if ((line != NULL) && (strncmp(line, "Uid:", 4) == 0)) {
            // get status_user_name
            char *s_uid = split(line, "	\n", 1);
            uid_t uid = strtol(s_uid, NULL, 10);
            struct passwd *p = getpwuid(uid);
            if (p != NULL) {
                *status_user_name = (char *) mymalloc(strlen(p->pw_name) + 1);
                strcpy(*status_user_name, p->pw_name);
            }
        } else if ((line != NULL) && (strncmp(line, "Name:", 5) == 0)) {
            // get command_name
            char *splitted = split(line, "	\n", 1);
            *command_name = (char *) mymalloc(strlen(splitted) + 1);
            strcpy(*command_name, splitted);
        }
    }
    free(line);

    myfclose(f);
}

void list_processes(const char *user_name) {
    if (user_name == NULL) {
        return;
    }

    DIR *dir = myopendir("/proc/", 1);

    // iterate through pids
    while (1) {
        struct dirent *dir_entry = myreaddir(dir, 1);
        if (dir_entry == NULL) {
            break;
        }

        char *pid = dir_entry->d_name;
        // check if it is a valid pid
        if (valid_integer(pid)) {
            char *status_user_name = NULL;
            char *command_name = NULL;
            get_status(pid, &status_user_name, &command_name);
            // check if user name from status file is equal to the target user
            // name
            if ((status_user_name != NULL) &&
                (strcmp(user_name, status_user_name) == 0)) {
                printf("pid=%s, command_name=%s\n", pid, command_name);
            }

            free(status_user_name);
            free(command_name);
        }
    }
}

int main(int argc, char *argv[]) {
    // opt vars
    int opt;
    char *user_name = NULL;

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
    if (optind == argc - 1) {
        user_name = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    list_processes(user_name);

    exit(EXIT_SUCCESS);
}
