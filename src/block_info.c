#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "dbg.h"
#include "endian.h"
#include "bstring.h"
#include "mem.h"
#include "block_info.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a block info object.
//
// Returns a reference to a new block info object if successful. Otherwise
// returns null.
sky_block_info *sky_block_info_create()
{
    sky_block_info *info = calloc(sizeof(sky_block_info), 1);
    check_mem(info);
    return info;
    
error:
    sky_block_info_free(info);
    return NULL;
}

// Removes a block info object from memory.
void sky_block_info_free(sky_block_info *info)
{
    if(info) {
        info->index = 0;
        info->min_object_id = 0;
        info->max_object_id = 0;
        info->min_timestamp = 0;
        info->max_timestamp = 0;
        info->spanned = false;
        free(info);
    }
}


//======================================
// Serialization
//======================================

// Packs a block info object into memory at a given pointer.
//
// info - The block info object to pack.
// ptr  - The pointer to the current location.
// sz   - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_info_pack(sky_block_info *info, void *ptr, size_t *sz)
{
    void *start = ptr;

    // Validate.
    check(info != NULL, "Block info required");
    check(ptr != NULL, "Pointer required");
    
    // Write object id range.
    sky_object_id_t min_object_id = htonll(info->min_object_id);
    sky_object_id_t max_object_id = htonll(info->max_object_id);
    memwrite(ptr, &min_object_id, sizeof(min_object_id), "min object id");
    memwrite(ptr, &max_object_id, sizeof(max_object_id), "max object id");

    // Write timestamp range.
    sky_timestamp_t min_timestamp = htonll(info->min_timestamp);
    sky_timestamp_t max_timestamp = htonll(info->max_timestamp);
    memwrite(ptr, &min_timestamp, sizeof(min_timestamp), "min timestamp");
    memwrite(ptr, &max_timestamp, sizeof(max_timestamp), "max timestamp");

    // Store number of bytes written.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    return -1;
}

// Unpacks a block info object from memory at the current pointer.
//
// info - The block info object to unpack into.
// ptr  - The pointer to the current location.
// sz   - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_info_unpack(sky_block_info *info, void *ptr, size_t *sz)
{
    void *start = ptr;
    
    // Validate.
    check(info != NULL, "Block info required");
    check(ptr != NULL, "Pointer required");

    // Write object id range.
    memread(ptr, &info->min_object_id, sizeof(info->min_object_id), "min object id");
    info->min_object_id = ntohll(info->min_object_id);
    memread(ptr, &info->max_object_id, sizeof(info->max_object_id), "max object id");
    info->max_object_id = ntohll(info->max_object_id);

    // Write timestamp range.
    memread(ptr, &info->min_timestamp, sizeof(info->min_timestamp), "min timestamp");
    info->min_timestamp = ntohll(info->min_timestamp);
    memread(ptr, &info->max_timestamp, sizeof(info->max_timestamp), "max timestamp");
    info->max_timestamp = ntohll(info->max_timestamp);

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}
