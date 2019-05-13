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
#include <ctype.h>

#define PROGRAM_NAME "pstree"

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

pid_t get_pid_max() {
    FILE *f = myfopen("/proc/sys/kernel/pid_max", "r", 1);
    char *line = (char *) mymalloc(512);
    mygetline(&line, f);
    pid_t pid_max = strtol(line, NULL, 10);
    free(line);
    return pid_max;
}

void get_status(const char *pid, char **ppid, char **command_name) {
    *ppid = 0;
    *command_name = NULL;

    // build status file path
    char *f_path = (char *) mymalloc(6 + strlen(pid) + 7 + 1);
    strcpy(f_path, "/proc/");
    strcpy(f_path + 6, pid);
    strcpy(f_path + strlen(f_path), "/status");

    FILE *f = myfopen(f_path, "r", 0);
    // process related to pid can be killed between readdir and this fopen
    if (f == NULL) {
        return;
    }
    free(f_path);

    // iterate through lines
    char *line = (char *) mymalloc(4096);
    while (1) {
        ssize_t read = mygetline(&line, f);
        if (read == -1) {
            // EOF
            break;
        }

        if ((line != NULL) && (strncmp(line, "PPid:", 5) == 0)) {
            // get ppid
            char *splitted = split(line, "	\n", 1);
            *ppid = (char *) mymalloc(strlen(splitted) + 1);
            strcpy(*ppid, splitted);
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

// Process tree
typedef struct ListNode ListNode;
struct ListNode {
    char *command_name;
    pid_t pid;
    ListNode *next;
};
pid_t pid_max;
ListNode **childs;

void init_tree() {
    pid_max = get_pid_max();
    childs = (ListNode **) mymalloc((pid_max + 1) * sizeof(ListNode *));
    for (pid_t pid = 0; pid <= pid_max; ++pid) {
        childs[pid] = NULL;
    }
}

void add_to_tree(const char *s_pid,
                 const char *s_ppid,
                 const char *command_name) {
    pid_t pid = strtol(s_pid, NULL, 10);
    pid_t ppid = strtol(s_ppid, NULL, 10);

    ListNode *node = (ListNode *) mymalloc(sizeof(ListNode));
    node->pid = pid;
    node->command_name = (char *) mymalloc(strlen(command_name) + 1);
    strcpy(node->command_name, command_name);

    if (childs[ppid] == NULL) {
        node->next = NULL;
    } else {
        node->next = childs[ppid];
    }
    childs[ppid] = node;
}

void free_tree() {
    for (pid_t pid = 0; pid <= pid_max; ++pid) {
        if (childs[pid] != NULL) {
            ListNode *node = childs[pid];
            while (node != NULL) {
                ListNode *node_next = node->next;
                free(node->command_name);
                free(node);
                node = node_next;
            }
        }
    }
    free(childs);
}

void print_node(ListNode *node, unsigned level) {
    for (unsigned i = 0; i < level * 4; ++i) {
        putchar(' ');
    }
    printf("%d (%s)\n", node->pid, node->command_name);
}

void dfs(pid_t pid, unsigned level) {
    ListNode *node = childs[pid];
    while (node != NULL) {
        print_node(node, level);
        dfs(node->pid, level + 1);
        node = node->next;
    }
}

void print_tree() {
    printf("0\n");
    dfs(0, 1);
}

void pstree() {
    DIR *dir = myopendir("/proc/", 1);

    init_tree();

    // iterate through pids
    while (1) {
        struct dirent *dir_entry = myreaddir(dir, 1);
        if (dir_entry == NULL) {
            break;
        }

        char *s_pid = dir_entry->d_name;
        // check if it is a valid pid
        if (valid_integer(s_pid)) {
            char *s_ppid = NULL;
            char *command_name = NULL;
            get_status(s_pid, &s_ppid, &command_name);
            if (s_ppid != NULL) {
                add_to_tree(s_pid, s_ppid, command_name);
            }
            free(s_ppid);
            free(command_name);
        }
    }

    print_tree();
    free_tree();
}

int main(int argc, char *argv[]) {
    // opt vars
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
    if (optind != argc) {
        usage(EXIT_FAILURE);
    }

    pstree();

    exit(EXIT_SUCCESS);
}
