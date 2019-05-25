#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PROGRAM_NAME "initgroups"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
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

int myinitgroups(const char *user, gid_t group) {
    gid_t *groups = (gid_t *) malloc((NGROUPS_MAX + 1) * sizeof(gid_t));
    if (groups == NULL) {
        return -1;
    }
    int cnt_groups = 0;

    groups[cnt_groups++] = group;

    errno = 0;
    struct passwd *pw = getpwnam(user);
    if (pw == NULL) {
        if (errno == ENOMEM) {
            free(groups);
            return -1;
        }
    } else {
        // get group from password file
        groups[cnt_groups++] = pw->pw_gid;

        // get groups from group file
        while (1) {
            errno = 0;
            struct group *grp = getgrent();
            if (grp == NULL) {
                if (errno == ENOMEM) {
                    free(groups);
                    return -1;
                } else {
                    break;
                }
            }

            for (int i = 0; grp->gr_mem[i] != NULL; ++i) {
                if (strcmp(grp->gr_mem[i], user) == 0) {
                    groups[cnt_groups++] = grp->gr_gid;
                    break;
                }
            }
        }
        endgrent();
    }

    int setgroups_ret = setgroups(cnt_groups, groups);
    if (setgroups_ret == -1) {
        if ((errno == ENOMEM) || (errno == EPERM)) {
            free(groups);
            return -1;
        }
    }

    free(groups);

    return 0;
}

void print_groups() {
    int size = NGROUPS_MAX + 1;
    gid_t *groups = (gid_t *) mymalloc(size * sizeof(gid_t));
    int num_groups = getgroups(size, groups);
    if (num_groups == -1) {
        error("getgroups");
    }
    for (int i = 0; i < num_groups; ++i) {
        printf("%d, ", groups[i]);
    }
    printf("\n");
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

    printf("before:\n");
    print_groups();

    if (myinitgroups("diego", 102) == -1) {
        error("myinitgroups");
    }

    printf("\nafter:\n");
    print_groups();

    exit(EXIT_SUCCESS);
}
