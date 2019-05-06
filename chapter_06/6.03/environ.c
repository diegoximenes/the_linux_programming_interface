#define _XOPEN_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>

#define PROGRAM_NAME "environ"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

extern char **environ;

void myclearenv() {
    environ = NULL;
}

void print_environ() {
    int i;
    printf("environ=\n");
    if (environ != NULL) {
        for (i = 0; environ[i] != NULL; ++i) {
            printf("%s\n", environ[i]);
        }
    }
}

int check_name(const char *name) {
    int i, len_name, has_equal;

    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }

    len_name = strlen(name);
    has_equal = 0;
    for (i = 0; !has_equal && (i < len_name); ++i) {
        if (name[i] == '=') {
            has_equal = 1;
        }
    }
    if (has_equal) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int mysetenv(const char *name, const char *value, int overwrite) {
    int valid_name, len_name, len_value, ret_putenv;
    char *p;

    valid_name = check_name(name);
    if (valid_name == -1) {
        return -1;
    }

    len_name = strlen(name);
    len_value = strlen(value);

    if ((getenv(name) != NULL) && (overwrite == 0)) {
        return 0;
    }

    p = (char *) malloc(len_name + len_value + 1);
    if (p == NULL) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(p, name, len_name);
    p[len_name] = '=';
    memcpy(p + len_name + 1, value, len_value);

    ret_putenv = putenv(p);
    if (ret_putenv < 0) {
        return -1;
    }

    return 0;
}

int env_with_same_name(const char *ep, const char *name) {
    int key_len;
    for (key_len = 0; ep[key_len] != '='; ++key_len);
    if (strncmp(ep, name, key_len) == 0) {
        return 1;
    }
    return 0;
}

// can cause memory leak
int myunsetenv(const char *name) {
    int valid_name, cnt_environ = 0, i;

    valid_name = check_name(name);
    if (valid_name == -1) {
        return -1;
    }

    if (environ != NULL) {
        for (cnt_environ = 0; environ[cnt_environ] != NULL; ++cnt_environ);

        for (i = 0; i < cnt_environ; ++i) {
            if (env_with_same_name(environ[i], name)) {
                environ[i] = environ[cnt_environ - 1];
                environ[cnt_environ - 1] = NULL;
                cnt_environ--;
            }
        }
    }

    return 0;
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

    myclearenv();

    mysetenv("key1", "val1", 0);
    mysetenv("key2", "val2", 0);
    mysetenv("key1", "val11", 1);
    mysetenv("key1", "val111", 0);
    mysetenv("key3=", "key3", 0);
    mysetenv("key4", "=key4", 0);

    print_environ();
    printf("\n");

    printf("getenv(key1)=%s\n", getenv("key1"));
    printf("getenv(key2)=%s\n", getenv("key2"));
    printf("getenv(key3=)=%s\n", getenv("key3="));
    printf("getenv(key4)=%s\n", getenv("key4"));
    printf("\n");

    myunsetenv("nokey");
    myunsetenv("key1");
    print_environ();
    printf("\n");

    myunsetenv("key2");
    myunsetenv("key4");
    myunsetenv("key4");
    print_environ();

    exit(EXIT_SUCCESS);
}
