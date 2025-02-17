#include <stdlib.h>
#include <string.h>
#include "cpen212alloc.h"
#include "cpen212common.h"
#include <assert.h>

// CHANGE THIS FILE to implement your heap allocator for each task

/* 
Heap Memory Layout:

Heap Size ((sizeof(size_t) bytes) <--heap handle points here
Block Header (sizeof(blockHeader) bytes) (8 for now)
User-Usable Space (8 byte aligned)
Footer (sizeof(size_t) bytes)
...more blocks

Block Structure:
1. Header (blockHeader struct):
    -size field (size_t): Contains both size and allocation status
    -least significant bit used as allocated/free flag (1 = allocated, 0 = free)
    -actual block size stored in upper bits (masked with BLOCK_SIZE_MASK)
    -size includes usable space header and footer
2. User-Usable Space:
    -starts immediately after the header
    -8-byte aligned for proper memory alignment
3. Footer:
    -size field (size_t): Contains the same value as the header
    -placed at the end of the block
*/

// void *cpen212_init(void *heap_start, void *heap_end) {
//     *((void **) heap_start) = heap_start + sizeof(void *);
//     return heap_start;
// }

void *cpen212_init(void *heap_start, void *heap_end) {
    if (!heap_start || !heap_end || heap_start >= heap_end) {
        return NULL; // Invalid heap boundaries
    }

    //store heap size at the beginning of the heap
    size_t heap_size = (size_t)((char *)heap_end - (char *)heap_start);
    *((size_t *)heap_start) = heap_size;

    //initialize first block (after the heap size)
    blockHeader *firstBlock = (blockHeader *)((char *)heap_start + sizeof(size_t));
    firstBlock->size = heap_size - sizeof(size_t);  //size includes header and payload
    setBlockAllocated(firstBlock, false);
    setBlockFooter(firstBlock); //set footer for first block

    return heap_start; //return start of heap
}

// this alloc is broken: it blithely allocates past the end of the heap
// void *cpen212_alloc(void *heap_handle, size_t nbytes) {
//     size_t aligned_sz = (nbytes + 7) & ~7;
//     void *p = *((void **) heap_handle);
//     *((void **) heap_handle) += aligned_sz;
//     return p;
// }
void *cpen212_alloc(void *heap_handle, size_t nbytes) {
    if (!heap_handle || nbytes == 0) {
        return NULL;
    }

    //retrieve heap size (stored at the beginning of the heap)
    size_t heap_size = *((size_t *)heap_handle);

    //start from first block (after the heap size)
    blockHeader *current = (blockHeader *)((char *)heap_handle + sizeof(size_t));

    //make sure the requested size is 8-byte aligned
    size_t alignedSize = (nbytes + 7) & ~7;

    //calculate total size needed (payload + header)
    size_t totalSize = alignedSize + sizeof(blockHeader) + sizeof(size_t); //add size for footer

    //traverse heap linearly
    while ((char *)current < (char *)heap_handle + heap_size) {
        if (!isBlockAllocated(current) && getBlockSize(current) >= totalSize) {
            size_t remainingSize = getBlockSize(current) - totalSize;

            //check if remaining space is big enough to create a new block
            if (remainingSize >= sizeof(blockHeader) + sizeof(size_t)) {
                //splitting
                blockHeader *newBlock = (blockHeader *)((char *)current + totalSize);
                newBlock->size = remainingSize;
                setBlockAllocated(newBlock, false);
                setBlockFooter(newBlock);

                //update the current block's size
                current->size = totalSize;
            } else {
                //if remaining space is too small use the entire block
                totalSize = getBlockSize(current);
            }

            //set current block as allocated
            setBlockAllocated(current, true);
            setBlockFooter(current);
            //return address of the usable space (after the block header)
            return (void *)((char *)current + sizeof(blockHeader));
        }

        //move to next block
        current = (blockHeader *)((char *)current + getBlockSize(current));
    }

    //no sufficient free block found
    return NULL;
}

void cpen212_free(void *heap_handle, void *p) {
    //validate input parameters
    if (!heap_handle || !p) {
        return;
    }

    //get block header by moving back sizeof(blockHeader) bytes from user pointer
    blockHeader *block = (blockHeader *)((char *)p - sizeof(blockHeader));
    setBlockAllocated(block, false);    //mark block as free (unallocated)

    //get heap size for boundary checking
    size_t heap_size = getHeapSize(heap_handle);

    //backwards coalescing - check if previous block exists and is free
    if ((char *)block > (char *)heap_handle + sizeof(size_t)) {
        // Get previous block's footer
        size_t *prevFooter = (size_t *)((char *)block - sizeof(size_t));
        
        // If previous block exists and is free
        if (!(*prevFooter & BLOCK_ALLOCATED)) {
            size_t prevSize = *prevFooter & BLOCK_SIZE_MASK;
            blockHeader *prevBlock = (blockHeader *)((char *)block - prevSize);
            
            // Update previous block's size to include current block
            prevBlock->size = getBlockSize(prevBlock) + getBlockSize(block);
            block = prevBlock; // Update block pointer for forward coalescing
        }
    }

    //forward coalescing - check if next block exists and is free
    blockHeader *nextBlock = (blockHeader *)((char *)block + getBlockSize(block));  //get next block
    
    //check if next block is within heap bounds
    if ((char *)nextBlock < (char *)heap_handle + heap_size) {
        //if next block is free merge w current block
        if (!isBlockAllocated(nextBlock)) {
            //update current block size to include next block
            block->size = getBlockSize(block) + getBlockSize(nextBlock);
            
        }
    }
    setBlockFooter(block);  //update footer after coalescing
}

void *cpen212_realloc(void *heap_handle, void *prev, size_t nbytes) {
    if (!heap_handle) { //validate heap handle
        return NULL;    
    }

    //if prev == NULL treat as new allocation
    if (!prev) {
        return cpen212_alloc(heap_handle, nbytes);
    }

    //get old block header and its size
    blockHeader *oldBlock = (blockHeader *)((char *)prev - sizeof(blockHeader));
    size_t oldSize = getBlockSize(oldBlock) - sizeof(blockHeader) - sizeof(size_t);

    //allocate new block of requested size
    void *newBlock = cpen212_alloc(heap_handle, nbytes);
    if (!newBlock) {
        return NULL;    //allocation failed
    }

    //copy data from old block to new block
    //use smaller of old and new sizes to prevent buffer overflow
    size_t copySize = (oldSize < nbytes) ? oldSize : nbytes;
    memcpy(newBlock, prev, copySize);

    //free the old block
    cpen212_free(heap_handle, prev);

    return newBlock;    //return pointer to new block
}
