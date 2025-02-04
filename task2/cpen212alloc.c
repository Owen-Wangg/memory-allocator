#include <stdlib.h>
#include <string.h>
#include "cpen212alloc.h"
#include "cpen212common.h"

// CHANGE THIS FILE to implement your heap allocator for each task

void *cpen212_init(void *heap_start, void *heap_end) {
    *((void **) heap_start) = heap_start + sizeof(void *);
    return heap_start;
}

// this alloc is broken: it blithely allocates past the end of the heap
void *cpen212_alloc(void *heap_handle, size_t nbytes) {
    size_t aligned_sz = (nbytes + 7) & ~7;
    void *p = *((void **) heap_handle);
    *((void **) heap_handle) += aligned_sz;
    return p;
}


void cpen212_free(void *s, void *p) {
}

void *cpen212_realloc(void *s, void *prev, size_t nbytes) {
    return NULL;
}
