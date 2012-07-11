#ifndef _block_h
#define _block_h

#include <stddef.h>
#include <inttypes.h>

#include "table.h"
#include "path.h"
#include "event.h"
#include "data_file.h"


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

// The maximum size the header can be.
#define MAX_BLOCK_HEADER_LENGTH 9


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct sky_block {
    sky_data_file *data_file;
    sky_block_info *info;
    uint32_t path_count;
    sky_path **paths;
} sky_block;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_block *sky_block_create();

void sky_block_free(sky_block *block);


//======================================
// Serialization
//======================================

size_t sky_block_sizeof(sky_block *block);

size_t sky_block_sizeof_raw_hdr(void *ptr);

int sky_block_pack(sky_block *block, void *addr, size_t *length);

int sky_block_unpack(sky_block *block, void *addr, size_t *length);


//======================================
// Block Info
//======================================

int sky_block_update_info(sky_block *block);


//======================================
// Event Management
//======================================

int sky_block_add_event(sky_block *block, sky_event *event);

int sky_block_remove_event(sky_block *block, sky_event *event);


//======================================
// Path Management
//======================================

int sky_block_add_path(sky_block *block, sky_path *path);

int sky_block_remove_path(sky_block *block, sky_path *path);

#endif
