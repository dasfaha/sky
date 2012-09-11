#ifndef _qip_mempool_h
#define _qip_mempool_h

#include <inttypes.h>

//==============================================================================
//
// Definitions
//
//==============================================================================

// Default to 64k memory blocks.
#define QIP_MEMPOOL_BLOCK_SIZE 0x10000

// Default to 8 byte alignment.
#define QIP_MEMPOOL_ALIGN_SIZE 8

// The memory pool maintains a list of blocks that hold memory in the heap.
// Each new allocation returns a pointer at the end of the last allocation.
// New blocks are added automatically as existing blocks are used up.
typedef struct {
    uint32_t block_count;
    void **blocks;
    void *ptr;
} qip_mempool;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_mempool *qip_mempool_create();

void qip_mempool_free(qip_mempool *mempool);

void qip_mempool_free_blocks(qip_mempool *mempool);


//--------------------------------------
// Allocation
//--------------------------------------

int qip_mempool_malloc(qip_mempool *mempool, size_t sz, void **ptr);

int qip_mempool_reset(qip_mempool *mempool);

#endif
