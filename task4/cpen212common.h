#ifndef __CPEN212COMMON_H__
#define __CPEN212COMMON_H__

// YOUR CODE HERE
//
// This file is included in cpen212alloc.c and cpen212debug.c,
// so it would be the right place to define data types they both share.

/*
The blockHeader struct represents the metadata for each memory block.
It is placed immediately before the user-usable space of each block.
*/
typedef struct blockHeader {
    size_t size; 
} __attribute__((aligned(8)))blockHeader;

#define BLOCK_ALLOCATED  ((size_t)1) // Use least significant bit
#define BLOCK_SIZE_MASK  (~BLOCK_ALLOCATED) // Mask to extract actual size

static inline size_t getBlockSize(blockHeader *block) {
    return block->size & BLOCK_SIZE_MASK;
}

static inline bool isBlockAllocated(blockHeader *block) {
    return (block->size & BLOCK_ALLOCATED) != 0;
}

static inline void setBlockAllocated(blockHeader *block, bool allocated) {
    if(allocated) {
        block->size |= BLOCK_ALLOCATED;
    }
    else {
        block->size &= BLOCK_SIZE_MASK;
    }
}

static inline size_t getHeapSize(void *heap_handle) {
    return *((size_t *)heap_handle);
}

static inline size_t *getBlockFooter(blockHeader *block) {
    return (size_t *)((char *)block + getBlockSize(block) - sizeof(size_t));
}

static inline void setBlockFooter(blockHeader *block) {
    size_t *footer = getBlockFooter(block);
    *footer = block->size;
}

static inline blockHeader *getPrevBlock(blockHeader *block) {
    size_t *prevFooter = (size_t *)((char *)block - sizeof(size_t));
    size_t prevSize = *prevFooter & BLOCK_SIZE_MASK;
    return (blockHeader *)((char *)block - prevSize);
}

#endif // __CPEN212COMMON_H__
