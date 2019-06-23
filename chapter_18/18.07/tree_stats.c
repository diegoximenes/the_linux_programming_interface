#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <ftw.h>

#define PROGRAM_NAME "tree_stats"

#define UNUSED(x) (void)(x)

void usage(int status) {
    printf("Usage: %s DIR_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

void mynftw(const char *dirpath,
            int (*fn) (const char *fpath,
                       const struct stat *sb,
                       int typeflag,
                       struct FTW *ftwbuf),
            int nopenfd,
            int flags) {
    if (nftw(dirpath, fn, nopenfd, flags) == -1) {
        error("nftw");
    }
}

unsigned reg = 0;
unsigned dir = 0;
unsigned chr = 0;
unsigned blk = 0;
unsigned lnk = 0;
unsigned ifo = 0;
unsigned sock = 0;

int handle_file(const char *pathname,
                const struct stat *statbuf,
                int typeflag,
                struct FTW *ftwbuf) {
    UNUSED(pathname);
    UNUSED(typeflag);
    UNUSED(ftwbuf);

    switch (statbuf->st_mode & S_IFMT) {
        case S_IFREG:
            reg++;
            break;
        case S_IFDIR:
            dir++;
            break;
        case S_IFCHR:
            chr++;
            break;
        case S_IFBLK:
            blk++;
            break;
        case S_IFLNK:
            lnk++;
            break;
        case S_IFIFO:
            ifo++;
            break;
        case S_IFSOCK:
            sock++;
        default:
            break;
    }

    return 0;
}

void tree_stats(const char *dir_path) {
    mynftw(dir_path, handle_file, 10, 0);
    printf("reg=%u\n", reg);
    printf("dir=%u\n", dir);
    printf("chr=%u\n", chr);
    printf("blk=%u\n", blk);
    printf("lnk=%u\n", lnk);
    printf("ifo=%u\n", ifo);
    printf("sock=%u\n", sock);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *dir_path = NULL;

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
        dir_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    tree_stats(dir_path);

    exit(EXIT_SUCCESS);
}
