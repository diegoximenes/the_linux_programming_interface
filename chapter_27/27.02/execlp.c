#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define PROGRAM_NAME "execlp"

void usage(int status) {
    printf("Usage: %s EXEC_PATH BASE_NAME ARG0(OPTIONAL) ARG1(OPTIONAL)\n",
           PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void free_exec(char **p, unsigned len) {
    for (unsigned i = 0; i < len; ++i) {
        free(p[i]);
    }
    free(p);
}

int myexeclp(const char *file, const char *arg, ... /* (char *) 0 */) {
    va_list ap;

    // count number of args
    unsigned number_of_args = 0;
    va_start(ap, arg);
    for (const char *curr_arg = arg;
         curr_arg != (char *) 0;
         curr_arg = va_arg(ap, char *)) {

        number_of_args++;
    }
    va_end(ap);

    // allocate argv
    char **argv = (char **) malloc((number_of_args + 1) * sizeof(char *));
    if (argv == NULL) {
        return -1;
    }

    // set argv
    argv[number_of_args] = NULL;
    unsigned idx_args = 0;
    va_start(ap, arg);
    for (const char *curr_arg = arg;
         curr_arg != (char *) 0;
         curr_arg = va_arg(ap, char*)) {

        argv[idx_args] = (char *) malloc(1 + strlen(curr_arg));
        // failed to allocate, free resources
        if (argv[idx_args] == NULL) {
            va_end(ap);
            free_exec(argv, idx_args);
            return -1;
        }

        strcpy(argv[idx_args], curr_arg);
        idx_args++;
    }
    va_end(ap);

    extern char **environ;

    int ret;
    if (strchr(file, '/') == NULL) {
        // file is only a file name, tries to find file in one of the paths in
        // PATH env var

        char *path = (char *) malloc(PATH_MAX);
        if (path == NULL) {
            free_exec(argv, number_of_args);
            return -1;
        }

        size_t len_file = strlen(file);

        char *env_paths = getenv("PATH");
        char delim[] = ":";
        char *env_path = strtok(env_paths, delim);
        while (env_path != NULL) {
            size_t len_env_path = strlen(env_path);
            if (len_env_path + 1 + len_file + 1 > PATH_MAX) {
                free(path);
                free_exec(argv, number_of_args);
                errno = ENAMETOOLONG;
                return -1;
            }
            strcpy(path, env_path);
            path[len_env_path] = '/';
            strcpy(path + len_env_path + 1, file);

            execve(path, argv, environ);

            env_path = strtok(NULL, delim);
        }

        free(path);
        errno = ENOENT;
        ret = -1;
    } else {
        // file is going to handled as a path
        ret = execve(file, argv, environ);
    }

    // some error occurred, free resources
    free_exec(argv, number_of_args);

    return ret;
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *exec_file = NULL;
    char *base_name = NULL;
    char *arg0 = 0;
    char *arg1 = 0;

    // parse argv
    enum {
        GETOPT_HELP_CHAR = (CHAR_MIN - 2),
    };
    static struct option const long_options[] = {
        {"help", no_argument, NULL, GETOPT_HELP_CHAR},
    };
    while ((opt = getopt_long(argc, argv, "a", long_options, NULL)) != -1) {
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
        exec_file = argv[optind++];
        base_name = argv[optind++];
    } else if(optind == argc - 3) {
        exec_file = argv[optind++];
        base_name = argv[optind++];
        arg0 = argv[optind++];
    } else if(optind == argc - 4) {
        exec_file = argv[optind++];
        base_name = argv[optind++];
        arg0 = argv[optind++];
        arg1 = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    myexeclp(exec_file, base_name, arg0, arg1, 0);
    perror("myexeclp");

    exit(EXIT_SUCCESS);
}
