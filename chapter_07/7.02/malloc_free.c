#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define PROGRAM_NAME "malloc_free"

void usage(int status) {
    printf("Usage: %s\n", PROGRAM_NAME);
    exit(status);
}

// just a supposition
unsigned word_size = __WORDSIZE / 8;

// Pointer comparison in c is undefined when the operands belong to different
// "objects", e.g., pointers returned by different calls to sbrk.
int cmp_ptr(const void *ptr1, const void *ptr2) {
    uintptr_t addr1 = (uintptr_t) ptr1;
    uintptr_t addr2 = (uintptr_t) ptr2;
    if (addr1 < addr2) {
        return -1;
    } else if (addr1 == addr2) {
        return 0;
    } else {
        return 1;
    }
}

// increment ptr by inc bytes
void *delta_ptr(const void *ptr, intptr_t inc) {
    uintptr_t addr = (uintptr_t) ptr;
    return (void *) (addr + inc);
}

// Doubly linked list that keeps free memory blocks. Sorted by block address.
typedef struct FreeListNode FreeListNode;
struct FreeListNode {
    size_t block_size;
    FreeListNode *next, *prev;
};
FreeListNode *free_list_head = NULL;

void remove_from_free_list(const FreeListNode *node) {
    if (node == free_list_head) {
        free_list_head = node->next;
        if (node->next != NULL) {
            node->next->prev = NULL;
        }
    } else {
        node->prev->next = node->next;
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
    }
}

// If left and right are adjacent blocks in memory then they are merged.
// Returns the node with largest address resulted in the merge operation.
// Considers that address of left is smaller than of right,
// and that left and right are not NULL.
FreeListNode *merge_nodes(FreeListNode *left, FreeListNode *right) {
    if (cmp_ptr(delta_ptr(left, left->block_size), right) == 0) {
        left->block_size += right->block_size;
        left->next = right->next;
        if (right->next != NULL) {
            right->next->prev = left;
        }
        return left;
    }
    return right;
}

void add_to_free_list(void *ptr, size_t block_size) {
    if (free_list_head == NULL) {
        // empty list, ptr is the new head
        free_list_head = (FreeListNode *) ptr;
        free_list_head->block_size = block_size;
        free_list_head->next = free_list_head->prev = NULL;
    } else {
        // get node with largest address that is smaller than ptr
        FreeListNode *node = free_list_head, *last_node_smaller = NULL;
        while (node != NULL) {
            if (cmp_ptr(node, ptr) == 1) {
                break;
            }
            last_node_smaller = node;
            node = node->next;
        }

        // no node smaller than ptr, ptr will be the new head
        if (last_node_smaller == NULL) {
            node = (FreeListNode *) ptr;
            node->block_size = block_size;
            node->next = free_list_head;
            node->prev = NULL;
            if (free_list_head != NULL) {
                free_list_head->prev = node;
            }
            free_list_head = node;
            merge_nodes(node, node->next);
        } else {
            node = (FreeListNode *) ptr;
            node->block_size = block_size;
            node->next = last_node_smaller->next;
            if (node->next != NULL) {
                node->next->prev = node;
            }
            node->prev = last_node_smaller;
            last_node_smaller->next = node;
            node = merge_nodes(last_node_smaller, node);
            if (node->next != NULL) {
                merge_nodes(node, node->next);
            }
        }
    }
}

// set program break to a multiple of word size
void align_program_break() {
    uintptr_t addr = (uintptr_t) sbrk(0);
    uintptr_t inc = (word_size - addr % word_size) % word_size;
    sbrk(inc);
}

void *mymalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    align_program_break();

    size_t block_size = sizeof(size_t) + size;
    // guarantees that the new block has enough size to insert it in the
    // free block list
    if (sizeof(FreeListNode) > block_size) {
        block_size = sizeof(FreeListNode);
    }
    // align block
    block_size += (word_size - block_size % word_size) % word_size;

    // search for first fit in free list
    FreeListNode *node = free_list_head, *first_fit = NULL;
    while (node != NULL) {
        if (node->block_size >= block_size) {
            first_fit = node;
            break;
        }
        node = node->next;
    }

    // requests more memory to the kernel if necessary
    if (first_fit == NULL) {
        void *ptr = sbrk(block_size);
        if (ptr == (void *) -1) {
            return NULL;
        }
        memcpy(ptr, &block_size, sizeof(block_size));
        return (void *) delta_ptr(ptr, sizeof(block_size));
    }

    // uses memory allocated in previous calls to mymalloc
    remove_from_free_list(first_fit);
    if (first_fit->block_size > block_size) {
        size_t free_block_size = first_fit->block_size - block_size;
        if (free_block_size >= sizeof(FreeListNode)) {
            // first fit has size larger than necessary,
            // then adds the rest to the free list
            add_to_free_list(delta_ptr(first_fit, block_size), free_block_size);
        } else {
            block_size = first_fit->block_size;
        }
    }
    memcpy(first_fit, &block_size, sizeof(block_size));
    return delta_ptr(first_fit, sizeof(block_size));
}

void myfree(void *ptr) {
    void *block_ptr = delta_ptr(ptr, -sizeof(size_t));
    size_t *block_size = (size_t *) block_ptr;
    if (cmp_ptr(delta_ptr(block_ptr, *block_size), sbrk(0)) == 0) {
        // returns memory to kernel if block is the top of heap segment
        sbrk(-*block_size);
    } else {
        add_to_free_list(block_ptr, *block_size);
    }
}

void print_free_list() {
    printf("free_list:\n");
    if (free_list_head == NULL) {
        printf("empty\n");
    } else {
        FreeListNode *node = free_list_head;
        while (node != NULL) {
            printf("node=%p, block_size=%ld, prev=%p, next=%p\n",
                   node, node->block_size, node->prev, node->next);
            node = node->next;
        }
    }
}

void print_mymalloc_block(const void *ptr) {
    void *block_ptr = delta_ptr(ptr, -sizeof(size_t));
    size_t *block_size = (size_t *) block_ptr;
    printf("block_ptr=%p, mymalloc_block_ptr=%p, block_size=%ld\n",
           block_ptr, ptr, *block_size);
}

int *test_allocation(int n) {
    int *p = (int *) mymalloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i) {
        p[i] = i;
    }
    printf("p= ");
    for (int i = 0; i < n; ++i) {
        printf("%d, ", p[i]);
    }
    printf("\n");
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

    printf("before\n");
    printf("program_break=%p\n", sbrk(0));
    printf("\n");

    printf("test 1\n");
    int *p1 = test_allocation(10);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 2\n");
    int *p2 = test_allocation(14);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p2: ");
    print_mymalloc_block(p2);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 3\n");
    int *p3 = test_allocation(1);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p2: ");
    print_mymalloc_block(p2);
    printf("block p3: ");
    print_mymalloc_block(p3);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 4\n");
    int *p4 = test_allocation(10);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p2: ");
    print_mymalloc_block(p2);
    printf("block p3: ");
    print_mymalloc_block(p3);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 5\n");
    myfree(p2);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p3: ");
    print_mymalloc_block(p3);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 6\n");
    myfree(p3);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 7\n");
    int *p7 = test_allocation(12);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("block p7: ");
    print_mymalloc_block(p7);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 8\n");
    int *p8 = test_allocation(50);
    printf("block p1: ");
    print_mymalloc_block(p1);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("block p7: ");
    print_mymalloc_block(p7);
    printf("block p8: ");
    print_mymalloc_block(p8);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 9\n");
    myfree(p1);
    printf("block p4: ");
    print_mymalloc_block(p4);
    printf("block p7: ");
    print_mymalloc_block(p7);
    printf("block p8: ");
    print_mymalloc_block(p8);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 10\n");
    myfree(p4);
    printf("block p7: ");
    print_mymalloc_block(p7);
    printf("block p8: ");
    print_mymalloc_block(p8);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    printf("test 11\n");
    int *p9 = test_allocation(15);
    printf("block p7: ");
    print_mymalloc_block(p7);
    printf("block p8: ");
    print_mymalloc_block(p8);
    printf("block p9: ");
    print_mymalloc_block(p9);
    printf("program_break=%p\n", sbrk(0));
    print_free_list();
    printf("\n");

    exit(EXIT_SUCCESS);
}
