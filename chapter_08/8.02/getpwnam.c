#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <pwd.h>
#include <string.h>

#define PROGRAM_NAME "getpwnam"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

struct passwd *mygetpwnam(const char *name) {
    struct passwd *p;
    while ((p = getpwent()) != NULL) {
        if (strcmp(name, p->pw_name) == 0) {
            break;
        }
    }
    endpwent();
    return p;
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

    struct passwd *p = mygetpwnam("mail");
    if (p != NULL) {
        printf("dir=%s,gecos=%s,gid=%d,name=%s,passwd=%s,shell=%s,uid=%d\n",
               p->pw_dir, p->pw_gecos, p->pw_gid, p->pw_name, p->pw_passwd,
               p->pw_shell, p->pw_uid);
    } else {
        printf("not found\n");
    }

    exit(EXIT_SUCCESS);
}
