#ifndef _block_h
#define _block_h

#include <stddef.h>
#include <inttypes.h>

#include "object_file.h"
#include "path.h"
#include "event.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// A block represents a contiguous area of storage for paths. A block can store
// multiple paths or paths can span across multiple blocks (block spanning).


//==============================================================================
//
// Constants
//
//==============================================================================

// The length of non-path data in the block.
#define BLOCK_HEADER_LENGTH sizeof(uint32_t)


//==============================================================================
//
// Typedefs
//
//==============================================================================

// The block stores an array of paths.
typedef struct Block {
    ObjectFile *object_file;
    BlockInfo *info;
    uint32_t path_count;
    Path **paths;
} Block;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Block *Block_create(ObjectFile *object_file, BlockInfo *info);

void Block_destroy(Block *block);


//======================================
// Serialization
//======================================

uint32_t Block_get_serialized_length(Block *block);

int Block_serialize(Block *block, void *addr, ptrdiff_t *length);

int Block_deserialize(Block *block, void *addr, ptrdiff_t *length);


//======================================
// Block Info
//======================================

int Block_update_info(Block *block);


//======================================
// Event Management
//======================================

int Block_add_event(Block *block, Event *event);

int Block_remove_event(Block *block, Event *event);


//======================================
// Path Management
//======================================

int Block_add_path(Block *block, Path *path);

int Block_remove_path(Block *block, Path *path);

#endif
