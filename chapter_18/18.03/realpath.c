#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#define PROGRAM_NAME "realpath"

void usage(int status) {
    printf("Usage: %s PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void free_realpath(int free_resolved_path,
                   char *resolved_path,
                   char *path_strtok,
                   char *link_content) {
    if (free_resolved_path) {
        free(resolved_path);
    }
    free(path_strtok);
    free(link_content);
}

int count_components(const char *path, size_t len) {
    // supposes that there are no consecutives '/'
    // supposes that the last charactere is not '/'
    int number_of_components = 0;
    for (size_t i = 0; i < len; ++i) {
        if (path[i] == '/') {
            number_of_components++;
        }
    }
    return number_of_components;
}

int cwd(char *resolved_path,
        int allocated_resolved_path,
        char *path_strtok,
        char *link_content,
        size_t *resolved_path_len) {
    if (getcwd(resolved_path, PATH_MAX) == NULL) {
        free_realpath(allocated_resolved_path,
                      resolved_path,
                      path_strtok,
                      link_content);
        return -1;
    }

    *resolved_path_len = strlen(resolved_path);
    return count_components(resolved_path, *resolved_path_len);
}

char *myrealpath(const char *path, char *resolved_path) {
    if (path == NULL) {
        errno = EINVAL;
        return NULL;
    }

    // allocate resolved_path if necessary
    int allocated_resolved_path = 0;
    if (resolved_path == NULL) {
        allocated_resolved_path = 1;
        resolved_path = (char *) malloc(PATH_MAX);
        if (resolved_path == NULL) {
            return NULL;
        }
    }

    // allocate auxiliary array to use strtok
    char *path_strtok = (char *) malloc(strlen(path) + 1);
    if (path_strtok == NULL) {
        free_realpath(allocated_resolved_path, resolved_path, NULL, NULL);
        return NULL;
    }
    strcpy(path_strtok, path);

    // allocate auxiliary array to use readlink
    char *link_content = (char *) malloc(PATH_MAX);
    if (link_content == NULL) {
        free_realpath(allocated_resolved_path,
                      resolved_path,
                      path_strtok,
                      NULL);
        return NULL;
    }

    // split path by '/'
    size_t resolved_path_len = 0;
    int last_component_not_dir = 0;
    int first_token = 1;
    int number_of_resolved_components = 0;
    const char *delim = "/";
    char *token = strtok(path_strtok, delim);
    while (token != NULL) {
        if (last_component_not_dir) {
            free_realpath(allocated_resolved_path,
                          resolved_path,
                          path_strtok,
                          link_content);
            errno = ENOTDIR;
            return NULL;
        }

        if (strcmp(token, ".") == 0) {
            // use cwd
            if (first_token && (path[0] != '/')) {
                number_of_resolved_components = cwd(resolved_path,
                                                    allocated_resolved_path,
                                                    path_strtok,
                                                    link_content,
                                                    &resolved_path_len);
                if (number_of_resolved_components == -1) {
                    return NULL;
                }
            }
        } else if (strcmp(token, "..") == 0) {
            // use cwd
            if (first_token && (path[0] != '/')) {
                number_of_resolved_components = cwd(resolved_path,
                                                    allocated_resolved_path,
                                                    path_strtok,
                                                    link_content,
                                                    &resolved_path_len);
                if (number_of_resolved_components == -1) {
                    return NULL;
                }
            }

            if (number_of_resolved_components != 0) {
                // remove last component
                number_of_resolved_components--;
                while ((resolved_path_len > 0) &&
                       (resolved_path[resolved_path_len - 1] != '/')) {
                    resolved_path[resolved_path_len - 1] = 0;
                    resolved_path_len--;
                }
                resolved_path[resolved_path_len - 1] = 0;
                resolved_path_len--;
            }
        } else {
            if (first_token && (path[0] != '/')) {
                number_of_resolved_components = cwd(resolved_path,
                                                    allocated_resolved_path,
                                                    path_strtok,
                                                    link_content,
                                                    &resolved_path_len);
                if (number_of_resolved_components == -1) {
                    return NULL;
                }
            }

            // append current component to resolved_path
            size_t token_len = strlen(token);
            if (resolved_path_len + 1 + token_len + 1 > PATH_MAX) {
                free_realpath(allocated_resolved_path,
                              resolved_path,
                              path_strtok,
                              link_content);
                errno = ENAMETOOLONG;
                return NULL;
            }
            resolved_path[resolved_path_len] = '/';
            resolved_path_len++;
            strcpy(resolved_path + resolved_path_len, token);
            resolved_path_len += strlen(token);
            number_of_resolved_components++;

            // follow symbolic link "recursively"
            while (1) {
                // get file stats
                struct stat status;
                if (lstat(resolved_path, &status) == -1) {
                    free_realpath(allocated_resolved_path,
                                  resolved_path,
                                  path_strtok,
                                  link_content);
                    return NULL;
                }

                if (!S_ISLNK(status.st_mode) && !S_ISDIR(status.st_mode)) {
                    last_component_not_dir = 1;
                }

                if (S_ISLNK(status.st_mode)) {
                    // from link to abs path
                    ssize_t return_readlink =
                        readlink(resolved_path, link_content, PATH_MAX);
                    if (return_readlink == -1) {
                        // don't need to worry with truncation, a PATH_MAX array
                        // guarantees that truncation will never occur
                        free_realpath(allocated_resolved_path,
                                      resolved_path,
                                      path_strtok,
                                      link_content);
                        return NULL;
                    }

                    // change last component of resolved_path to link_content
                    link_content[return_readlink] = 0;
                    size_t last_component_start;
                    for (last_component_start = resolved_path_len - 1;
                         last_component_start >= 0;
                         last_component_start--) {
                        if (resolved_path[last_component_start] == '/') {
                            last_component_start++;
                            break;
                        }
                    }
                    strcpy(resolved_path + last_component_start, link_content);
                    resolved_path_len = strlen(resolved_path);
                    number_of_resolved_components =
                        count_components(resolved_path, resolved_path_len);
                } else {
                    break;
                }
            }
        }

        token = strtok(NULL, delim);
        first_token = 0;
    }

    if (resolved_path_len == 0) {
        resolved_path[0] = '/';
        resolved_path[1] = 0;
    }

    free_realpath(0, NULL, path_strtok, link_content);

    return resolved_path;
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *path = NULL;

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
        path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    char *realpath = myrealpath(path, NULL);
    if (realpath == NULL) {
        error("myrealpath");
    }
    printf("%s\n", realpath);

    exit(EXIT_SUCCESS);
}
