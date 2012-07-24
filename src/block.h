#ifndef _block_h
#define _block_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_block sky_block;

#include "bstring.h"
#include "file.h"
#include "types.h"
#include "data_file.h"
#include "event.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// The block object stores information about the object id range and the
// timestamp range of each physical block in a data file. The block objects are
// stored in order of block index within the header file.
//
// The block also stores whether it is spanned, meaning that the
// object that it contains is stored across multiple blocks.


//==============================================================================
//
// Typedefs
//
//==============================================================================

#define SKY_BLOCK_HEADER_SIZE (sizeof(sky_object_id_t) * 2) + (sizeof(sky_timestamp_t) * 2)

struct sky_block {
    sky_data_file *data_file;
    uint32_t index;
    sky_object_id_t min_object_id;
    sky_object_id_t max_object_id;
    sky_timestamp_t min_timestamp;
    sky_timestamp_t max_timestamp;
    bool spanned;
};

// This structure is used for splitting blocks. It contains positional
// information about where a path in a block is located and also includes
// the future size of a path after a pending event is added.
typedef struct sky_block_path_stat {
    sky_object_id_t object_id;
    size_t start_pos;
    size_t end_pos;
    size_t sz;
} sky_block_path_stat;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_block *sky_block_create(sky_data_file *data_file);

void sky_block_free(sky_block *block);


//======================================
// Persistence
//======================================

int sky_block_pack(sky_block *block, void *ptr, size_t *sz);

int sky_block_unpack(sky_block *block, void *ptr, size_t *sz);


//======================================
// Header Management
//======================================

int sky_block_get_header_offset(sky_block *block, off_t *offset);

int sky_block_full_update(sky_block *block);


//======================================
// Block Position
//======================================

int sky_block_get_offset(sky_block *block, size_t *offset);

int sky_block_get_ptr(sky_block *block, void **ptr);


//======================================
// Spanning
//======================================

int sky_block_get_span_count(sky_block *block, uint32_t *count);


//======================================
// Stats
//======================================

int sky_block_get_path_stats(sky_block *block, sky_event *event,
    sky_block_path_stat **paths, uint32_t *path_count);


//======================================
// Event Management
//======================================

int sky_block_add_event(sky_block *block, sky_event *event);


//======================================
// Debugging
//======================================

int sky_block_memdump(sky_block *block);


#endif
