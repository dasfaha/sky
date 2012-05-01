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
// Definitions
//
//==============================================================================

// Stores the number of paths that a block contains.
#define sky_path_count_t uint32_t

// The length of non-path data in the block.
#define BLOCK_HEADER_LENGTH sizeof(sky_path_count_t)


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_block {
    ObjectFile *object_file;
    BlockInfo *info;
    sky_path_count_t path_count;
    Path **paths;
} sky_block;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_block *sky_block_create(ObjectFile *object_file, BlockInfo *info);

void sky_block_free(sky_block *block);


//======================================
// Serialization
//======================================

uint32_t sky_block_get_serialized_length(sky_block *block);

int sky_block_serialize(sky_block *block, void *addr, ptrdiff_t *length);

int sky_block_deserialize(sky_block *block, void *addr, ptrdiff_t *length);


//======================================
// Block Info
//======================================

int sky_block_update_info(sky_block *block);


//======================================
// Event Management
//======================================

int sky_block_add_event(sky_block *block, Event *event);

int sky_block_remove_event(sky_block *block, Event *event);


//======================================
// Path Management
//======================================

int sky_block_add_path(sky_block *block, Path *path);

int sky_block_remove_path(sky_block *block, Path *path);

#endif
