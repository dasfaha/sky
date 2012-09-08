#include <stdlib.h>

#include "mempool.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a memory pool.
qip_mempool *qip_mempool_create()
{
    qip_mempool *mempool = malloc(sizeof(qip_mempool));
    check_mem(mempool);
    mempool->blocks = NULL;
    mempool->block_count = 0;
    mempool->ptr = NULL;
    return mempool;
    
error:
    qip_mempool_free(mempool);
    return NULL;
}

// Frees an mempool.
//
// mempool - The mempool to free.
//
// Returns nothing.
void qip_mempool_free(qip_mempool *mempool)
{
    if(mempool) {
        qip_mempool_free_blocks(mempool);
        free(mempool);
    }
}

// Frees the blocks in a memory pool.
//
// mempool - The memory pool.
//
// Returns nothing.
void qip_mempool_free_blocks(qip_mempool *mempool)
{
    if(mempool) {
        uint32_t i;
        for(i=0; i<mempool->block_count; i++) {
            free(mempool->blocks[i]);
            mempool->blocks[i] = NULL;
        }
        if(mempool->blocks) free(mempool->blocks);
        mempool->blocks = NULL;
        mempool->block_count = 0;
        mempool->ptr = NULL;
    }
}


//======================================
// Allocation
//======================================

// Allocates a given number of bytes within the memory pool.
//
// mempool - The memory pool.
// size    - The number of bytes to allocate.
// ptr     - A pointer to where the allocated pointer should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_mempool_malloc(qip_mempool *mempool, size_t size, void **ptr)
{
    // Determine actual allocation size after alignment.
    size_t offset = (size % QIP_MEMPOOL_ALIGN_SIZE);
    if(offset > 0) {
        size += QIP_MEMPOOL_ALIGN_SIZE - offset;
    }
    
    // Validation.
    check(mempool != NULL, "Memory pool required");
    check(size > 0, "Allocation size must be greater than zero");
    check(size <= QIP_MEMPOOL_BLOCK_SIZE, "Allocation size (%ld) is greater than max size (%d)", size, QIP_MEMPOOL_BLOCK_SIZE);

    // If there are no blocks or there is not enough space left in the
    // current block then allocate a new block.
    if(mempool->block_count == 0 || (mempool->ptr + size) > (mempool->blocks[mempool->block_count-1] + QIP_MEMPOOL_BLOCK_SIZE)) {
        // Resize block list.
        mempool->block_count++;
        mempool->blocks = realloc(mempool->blocks, mempool->block_count * sizeof(*mempool->blocks));
        check_mem(mempool->blocks);

        // Allocate new block.
        mempool->blocks[mempool->block_count-1] = malloc(QIP_MEMPOOL_BLOCK_SIZE);
        check_mem(mempool->blocks[mempool->block_count-1]);
        
        // Move pointer to point at beginning of new block.
        mempool->ptr = mempool->blocks[mempool->block_count-1];
    }
    
    // Return a pointer to the current location.
    *ptr = mempool->ptr;
    
    // Move pointer by the number of bytes allocated.
    mempool->ptr += size;
    
    return 0;

error:
    *ptr = NULL;
    return -1;
}

// Resets the allocation pointer to the beginning of the first block. No
// memory in the pool is freed though.
//
// mempool - The memory pool.
//
// Returns 0 if successful, otherwise returns -1.
int qip_mempool_reset(qip_mempool *mempool)
{
    check(mempool != NULL, "Memory pool required");
    
    // Only update the allocation pointer if blocks exists. If no blocks exist
    // then the allocation pointer should be null anyway.
    if(mempool->block_count > 0) {
        mempool->ptr = mempool->blocks[0];
    }
    
    return 0;

error:
    return -1;
}

