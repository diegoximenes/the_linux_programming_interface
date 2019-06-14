#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include <sys/acl.h>
#include <acl/libacl.h>

#define PROGRAM_NAME "acl"

void usage(int status) {
    printf("Usage: %s [TYPE=u/g] ID FILE_PATH\n", PROGRAM_NAME);
    exit(status);
}

void error(const char *msg) {
    fprintf(stderr, "ERROR: ");
    perror(msg);
    exit(EXIT_FAILURE);
}

acl_t my_acl_get_file(const char *path_p, acl_type_t type) {
    acl_t ret = acl_get_file(path_p, type);
    if (ret == NULL) {
        error("acl_get_file");
    }
    return ret;
}

int my_acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry_p) {
    int ret = acl_get_entry(acl, entry_id, entry_p);
    if (ret == -1) {
        error("acl_get_entry");
    }
    return ret;
}

void my_acl_get_tag_type(acl_entry_t entry_d, acl_tag_t *tag_type_p) {
    if (acl_get_tag_type(entry_d, tag_type_p) == -1) {
        error("acl_get_tag_type");
    }
}

void my_acl_get_permset(acl_entry_t entry_d, acl_permset_t *permset_p) {
    if (acl_get_permset(entry_d, permset_p) == -1) {
        error("acl_get_permset");
    }
}

int my_acl_get_perm(acl_permset_t permset_d, acl_perm_t perm) {
    int ret = acl_get_perm(permset_d, perm);
    if (ret == -1) {
        error("acl_get_perm");
    }
    return ret;
}

void print_perm(int read, int write, int execute) {
    if (read) {
        printf("r");
    } else {
        printf("-");
    }

    if (write) {
        printf("w");
    } else {
        printf("-");
    }

    if (execute) {
        printf("x");
    } else {
        printf("-");
    }

    printf("\n");
}

void print_permission(const char *file_path, const char *type, const char *id) {
    // parse type and id
    int is_user;
    gid_t gid;
    uid_t uid;
    if (strcmp(type, "u") == 0) {
        is_user = 1;
        uid = atoi(id);
    } else if (strcmp(type, "g") == 0) {
        is_user = 0;
        gid = atoi(id);
    } else {
        usage(EXIT_FAILURE);
    }

    int found = 0;
    acl_permset_t permset, mask_permset;

    acl_t acl = my_acl_get_file(file_path, ACL_TYPE_ACCESS);
    acl_entry_t entry;
    int entry_status = my_acl_get_entry(acl, ACL_FIRST_ENTRY, &entry);
    while (entry_status != 0) {
        acl_tag_t tag_type;
        my_acl_get_tag_type(entry, &tag_type);

        void *qualifier = NULL;

        acl_permset_t curr_permset;
        my_acl_get_permset(entry, &curr_permset);

        switch (tag_type) {
            case ACL_USER:
                qualifier = acl_get_qualifier(entry);
                if (is_user &&
                        (qualifier != NULL) &&
                        (uid == * (uid_t *) qualifier)) {
                    permset = curr_permset;
                    found = 1;
                }
                break;
            case ACL_GROUP:
                qualifier = acl_get_qualifier(entry);
                if (!is_user &&
                        (qualifier != NULL) &&
                        (gid == * (gid_t *) qualifier)) {
                    permset = curr_permset;
                    found = 1;
                }
                break;
            case ACL_MASK:
                mask_permset = curr_permset;
                printf("mask: ");
                print_perm(my_acl_get_perm(mask_permset, ACL_READ),
                           my_acl_get_perm(mask_permset, ACL_WRITE),
                           my_acl_get_perm(mask_permset, ACL_EXECUTE));
                break;
            default:
                break;
        }

        acl_free(qualifier);

        entry_status = my_acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
    }

    if (found) {
        printf("perm: ");
        print_perm(my_acl_get_perm(permset, ACL_READ),
                   my_acl_get_perm(permset, ACL_WRITE),
                   my_acl_get_perm(permset, ACL_EXECUTE));

        printf("perm with mask: ");
        int perm_with_mask_read = my_acl_get_perm(permset, ACL_READ) &&
            my_acl_get_perm(mask_permset, ACL_READ);
        int perm_with_mask_write = my_acl_get_perm(permset, ACL_WRITE) &&
            my_acl_get_perm(mask_permset, ACL_WRITE);
        int perm_with_mask_execute = my_acl_get_perm(permset, ACL_EXECUTE) &&
            my_acl_get_perm(mask_permset, ACL_EXECUTE);
        print_perm(perm_with_mask_read,
                   perm_with_mask_write,
                   perm_with_mask_execute);
    }

    acl_free(acl);
}

int main(int argc, char *argv[]) {
    // getopt variables
    int opt;
    char *type = NULL, *file_path = NULL, *id = NULL;

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
        type = argv[optind++];
        id = argv[optind++];
        file_path = argv[optind++];
    } else {
        usage(EXIT_FAILURE);
    }

    print_permission(file_path, type, id);

    exit(EXIT_SUCCESS);
}
