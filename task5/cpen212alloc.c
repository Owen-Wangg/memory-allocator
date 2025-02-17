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
    -size includes the usable space header and footer
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
        return NULL; //invalid heap boundaries
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

            //check if remaining space is big enough to create new block
            if (remainingSize >= sizeof(blockHeader) + sizeof(size_t)) {
                //splitting
                blockHeader *newBlock = (blockHeader *)((char *)current + totalSize);
                newBlock->size = remainingSize;
                setBlockAllocated(newBlock, false);
                setBlockFooter(newBlock);

                //update the current block's size
                current->size = totalSize;
            } else {
                //if remaining space is too small use entire block
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
        //get previous block's footer
        size_t *prevFooter = (size_t *)((char *)block - sizeof(size_t));
        
        //if previous block exists and is free
        if (!(*prevFooter & BLOCK_ALLOCATED)) {
            size_t prevSize = *prevFooter & BLOCK_SIZE_MASK;
            blockHeader *prevBlock = (blockHeader *)((char *)block - prevSize);
            
            //update previous block's size to include current block
            prevBlock->size = getBlockSize(prevBlock) + getBlockSize(block);
            block = prevBlock; //update block pointer for forward coalescing
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

    //calc new total size needed (payload + header + footer)
    size_t alignedSize = (nbytes + 7) & ~7; //ensure 8-byte alignment
    size_t totalSize = alignedSize + sizeof(blockHeader) + sizeof(size_t);

    //check if block can be resized
    size_t currentSize = getBlockSize(oldBlock);
    if (totalSize <= currentSize) {
        //shrink block in place
        size_t remainingSize = currentSize - totalSize;

        if (remainingSize >= sizeof(blockHeader) + sizeof(size_t)) {
            //split block
            blockHeader *newBlock = (blockHeader *)((char *)oldBlock + totalSize);
            newBlock->size = remainingSize;
            setBlockAllocated(newBlock, false);
            setBlockFooter(newBlock);

            //update the old block size
            oldBlock->size = totalSize;
            setBlockFooter(oldBlock);
        }

        return prev; //return same pointer
    }

    //try to extend the block in place by coalescing with neighboring blocks
    blockHeader *nextBlock = (blockHeader *)((char *)oldBlock + getBlockSize(oldBlock));
    size_t heap_size = getHeapSize(heap_handle);

    //forward coalescing: check if next block is free and can merge
    if ((char *)nextBlock < (char *)heap_handle + heap_size && !isBlockAllocated(nextBlock)) {
        size_t combinedSize = getBlockSize(oldBlock) + getBlockSize(nextBlock);

        if (combinedSize >= totalSize) {
            //merge with next block
            oldBlock->size = combinedSize;
            setBlockFooter(oldBlock);

            //if remaining space split the block
            size_t remainingSize = combinedSize - totalSize;
            if (remainingSize >= sizeof(blockHeader) + sizeof(size_t)) {
                blockHeader *newBlock = (blockHeader *)((char *)oldBlock + totalSize);
                newBlock->size = remainingSize;
                setBlockAllocated(newBlock, false);
                setBlockFooter(newBlock);

                //update old block size
                oldBlock->size = totalSize;
                setBlockFooter(oldBlock);
            }

            return prev; //retrun same pointer
        }
    }

    //backward coalescing: check if previous block is free and can be merged
    if ((char *)oldBlock > (char *)heap_handle + sizeof(size_t)) {
        size_t *prevFooter = (size_t *)((char *)oldBlock - sizeof(size_t));
        if (!(*prevFooter & BLOCK_ALLOCATED)) {
            size_t prevSize = *prevFooter & BLOCK_SIZE_MASK;
            blockHeader *prevBlock = (blockHeader *)((char *)oldBlock - prevSize);

            size_t combinedSize = getBlockSize(prevBlock) + getBlockSize(oldBlock);

            if (combinedSize >= totalSize) {
                //merge w previous block
                prevBlock->size = combinedSize;
                setBlockFooter(prevBlock);

                //if remaining space split the block
                size_t remainingSize = combinedSize - totalSize;
                if (remainingSize >= sizeof(blockHeader) + sizeof(size_t)) {
                    blockHeader *newBlock = (blockHeader *)((char *)prevBlock + totalSize);
                    newBlock->size = remainingSize;
                    setBlockAllocated(newBlock, false);
                    setBlockFooter(newBlock);

                    //update prev block size
                    prevBlock->size = totalSize;
                    setBlockFooter(prevBlock);
                }

                return (void *)((char *)prevBlock + sizeof(blockHeader)); //return new pointer
            }
        }
    }

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