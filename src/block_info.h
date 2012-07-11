#ifndef _block_info_h
#define _block_info_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_block_info sky_block_info;

#include "bstring.h"
#include "file.h"
#include "types.h"

//==============================================================================
//
// Overview
//
//==============================================================================

// The block info object stores information about the object id range and the
// timestamp range of each block in a data file. The block info objects are
// stored in order of block index within the header file.
//
// The block info also stores whether a block is spanned, meaning that the
// object that it contains is stored across multiple blocks.


//==============================================================================
//
// Typedefs
//
//==============================================================================

#define SKY_BLOCK_INFO_SIZE (sizeof(sky_object_id_t) * 2) + (sizeof(sky_timestamp_t) * 2)

struct sky_block_info {
    uint32_t index;
    sky_object_id_t min_object_id;
    sky_object_id_t max_object_id;
    sky_timestamp_t min_timestamp;
    sky_timestamp_t max_timestamp;
    bool spanned;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_block_info *sky_block_info_create();

void sky_block_info_free(sky_block_info *info);


//======================================
// Persistence
//======================================

int sky_block_info_pack(sky_block_info *info, void *ptr, size_t *sz);

int sky_block_info_unpack(sky_block_info *info, void *ptr, size_t *sz);


#endif
