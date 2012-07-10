#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include "dbg.h"
#include "block.h"
#include "mem.h"
#include "minipack.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Path Sorting
//======================================

// Compares two paths and sorts them by object id.
int compare_paths(const void *_a, const void *_b)
{
    sky_path **a = (sky_path**)_a;
    sky_path **b = (sky_path**)_b;

    // Sort by object id.
    if((*a)->object_id > (*b)->object_id) {
        return 1;
    }
    else if((*a)->object_id < (*b)->object_id) {
        return -1;
    }
    else {
        return 0;
    }
}

// Sorts paths in a block.
//
// block - The block containing the paths.
void sort_paths(sky_block *block)
{
    qsort(block->paths, block->path_count, sizeof(sky_path*), compare_paths);
}


//======================================
// Lifecycle
//======================================

// Creates a reference to an in-memory block.
//
// table - The table that this block belongs to.
// info        - The header information about block.
//
// Returns a new block if successful, otherwise returns null.
sky_block *sky_block_create(sky_table *table, sky_block_info *info)
{
    sky_block *block;
    
    block = malloc(sizeof(sky_block)); check_mem(block);

    block->table = table;
    block->info = info;

    block->paths = NULL;
    block->path_count = 0;

    return block;
    
error:
    sky_block_free(block);
    return NULL;
}

// Removes a block reference from memory.
//
// block - The block to free.
void sky_block_free(sky_block *block)
{
    if(block) {
        block->table = NULL;
        block->info = NULL;

        // Destroy paths.
        uint32_t i=0;
        for(i=0; i<block->path_count; i++) {
            sky_path_free(block->paths[i]);
        }
        
        if(block->paths) free(block->paths);
        block->paths = NULL;
        block->path_count = 0;

        free(block);
    }
}


//======================================
// Serialization
//======================================

// Calculates the total number of bytes needed to store just the paths section
// of the block.
size_t get_paths_length(sky_block *block)
{
    size_t sz = 0;
    
    // Add size for each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        sz += sky_path_sizeof(block->paths[i]);
    }
    
    return sz;
}

// Calculates the total number of bytes needed to store a block and its paths.
// This number does not include the padding added after the block.
//
// block - The block.
size_t sky_block_sizeof(sky_block *block)
{
    size_t sz = 0;
    sz += minipack_sizeof_array(block->path_count);
    return sz;
}

// Calculates the number of bytes in the block header.
//
// ptr - A pointer to the raw block data.
//
// Returns the length of the raw block data.
size_t sky_block_sizeof_raw_hdr(void *ptr)
{
    size_t sz = 0;
    sz += minipack_sizeof_array_elem(ptr);
    return sz;
}    

// Serializes a block at a given memory location.
//
// block - The block to pack.
// ptr   - The pointer to the current location.
// sz    - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_pack(sky_block *block, void *ptr, size_t *sz)
{
    int rc;
    size_t _sz;
    void *start = ptr;

    // Validate.
    check(block != NULL, "Block required");
    check(ptr != NULL, "Pointer required");

    // Write path count.
    minipack_pack_int(ptr, block->path_count, &_sz);
    check(_sz != 0, "Unable to pack block path count");
    ptr += _sz;
    
    // Loop over paths and delegate serialization to each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        rc = sky_path_pack(block->paths[i], ptr, &_sz);
        check(rc == 0, "Unable to pack block path: %d", i);
        ptr += _sz;
    }
    
    // Null fill the rest of the block.
    int fillcount = block->table->block_size - (ptr-start);
    check(memset(ptr, 0, fillcount) != NULL, "Unable to null fill end of block");
    
    // Store number of bytes written.
    if(sz != NULL) {
        *sz = (size_t)(ptr-start);
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}

// Deserializes a block from a given memory location.
//
// block - The block to unpack.
// ptr   - The pointer to the current location.
// sz    - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_unpack(sky_block *block, void *ptr, size_t *sz)
{
    int rc;
    size_t _sz;
    void *start = ptr;

    // Validate.
    check(block != NULL, "Block required");
    check(ptr != NULL, "Pointer required");

    // Read path count.
    block->path_count = minipack_unpack_array(ptr, &_sz);
    check(_sz != 0, "Unable to unpack block path count");
    ptr += _sz;

    // Allocate paths.
    block->paths = realloc(block->paths, sizeof(sky_path*) * block->path_count);
    check_mem(block->paths);

    // Loop over paths and delegate deserialization to each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        sky_path *path = sky_path_create(0); check_mem(path);
        block->paths[i] = path;
        
        rc = sky_path_unpack(path, ptr, &_sz);
        check(rc == 0, "Unable to unpack block path: %d", i);
        ptr += _sz;
    }

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = (ptr-start);
    }

    return 0;

error:
    *sz = 0;
    return -1;
}


//======================================
// Block Info
//======================================

// Iterates over the paths and events in a block to find the min/max timestamp
// and object id and then updates the block info accordingly.
//
// block - The block to update.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_update_info(sky_block *block)
{
    sky_object_id_t min_object_id = 0;
    sky_object_id_t max_object_id = 0;
    sky_timestamp_t min_timestamp = INT64_MIN;
    sky_timestamp_t max_timestamp = INT64_MIN;
    
    // Validation.
    check(block != NULL, "Block required");

    // If there are no paths then clear ranges.
    if(block->path_count == 0) {
        min_object_id = max_object_id = 0;
        min_timestamp = max_timestamp = 0;
    }
    // Otherwise iterate over paths.
    else {
        uint32_t i, j;
        for(i=0; i<block->path_count; i++) {
            sky_path *path = block->paths[i];
        
            // Find object id range.
            if(min_object_id == 0 || path->object_id < min_object_id) {
                min_object_id = path->object_id;
            }
            if(max_object_id == 0 || path->object_id > max_object_id) {
                max_object_id = path->object_id;
            }
        
            // Iterate over events and find timstamp range.
            for(j=0; j<path->event_count; j++) {
                sky_event *event = path->events[j];
            
                // Find timestamp range.
                if(min_timestamp == INT64_MIN || event->timestamp < min_timestamp) {
                    min_timestamp = event->timestamp;
                }
                if(max_timestamp == INT64_MIN || event->timestamp > max_timestamp) {
                    max_timestamp = event->timestamp;
                }
            }
        }
    }
    
    // Update block info.
    sky_block_info *info = block->info;
    info->min_object_id = min_object_id;
    info->max_object_id = max_object_id;
    info->min_timestamp = min_timestamp;
    info->max_timestamp = max_timestamp;
    
    return 0;
    
error:
    return -1;
}


//======================================
// Event Management
//======================================

// Adds an event to an in-memory block. The event will automatically be inserted
// into an existing path if one exists with the same object id or a new path 
// will be created.
//
// block - The block to insert the event into.
// event - The event that is to be inserted.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_add_event(sky_block *block, sky_event *event)
{
    uint32_t i;
    int rc;
    
    // Validation.
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");
    check(event->object_id != 0, "Event object id cannot be 0");

    // Find existing path by object id.
    sky_path *path = NULL;
    for(i=0; i<block->path_count; i++) {
        if(block->paths[i]->object_id == event->object_id) {
            path = block->paths[i];
            break;
        }
    }
    
    // If matching path could not be found then create one.
    if(path == NULL) {
        path = sky_path_create(event->object_id); check_mem(path);
        rc = sky_block_add_path(block, path);
        check(rc == 0, "Unable to add new path to block")
    }
    
    // Add event to path.
    rc = sky_path_add_event(path, event);
    check(rc == 0, "Unable to add event to path");

    return 0;

error:
    return -1;
}

// Removes an event from a path in the block. If the event's path no longer has
// any events then it will automatically be removed too.
//
// block - The block that contains the event.
// event - The event that will be removed.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_remove_event(sky_block *block, sky_event *event)
{
    uint32_t i, j, index;
    int rc;
    
    // Validation.
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");
    check(event->object_id != 0, "Event object id cannot be 0");

    // Find event in paths.
    sky_path *path;

    for(i=0; i<block->path_count; i++) {
        bool found = false;
        path = block->paths[i];
        
        // If object id matches then search path.
        if(path->object_id == event->object_id) {
            for(j=0; j<path->event_count; j++) {
                // If event is found then remove it.
                if(path->events[j] == event) {
                    rc = sky_path_remove_event(path, event);
                    check(rc == 0, "Unable to remove event from path");
                    found = true;
                    break;
                }
            }
        }
        
        // Stop searching if the event was found.
        if(found) {
            index = i;
            break;
        }
    }
    
    // If path has no events then remove path from block.
    if(path != NULL && path->event_count == 0) {
        sky_block_remove_path(block, path);
    }

    return 0;

error:
    return -1;
}


//======================================
// Path Management
//======================================

// Adds an entire path to a block.
//
// block - The block that will contain the path.
// path  - The path to add.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_add_path(sky_block *block, sky_path *path)
{
    // Validation.
    check(block != NULL, "Block required");
    check(path != NULL, "Path required");
    check(path->object_id != 0, "Path object id cannot be 0");

    // Allocate space for path.
    block->path_count++;
    block->paths = realloc(block->paths, sizeof(sky_event*) * block->path_count);
    check_mem(block->paths);

    // Create and append path.
    block->paths[block->path_count-1] = path;

    // Sort paths.
    sort_paths(block);

    return 0;

error:
    return -1;
}

// Removes an entire path from a block.
//
// block - The block that contains the path.
// path  - The path in the block to remove.
//
// Returns 0 if successful, otherwise returns -1.
int sky_block_remove_path(sky_block *block, sky_path *path)
{
    uint32_t i, j;

    // Find index for path.
    for(i=0; i<block->path_count; i++) {
        if(block->paths[i] == path) {
            // Shift paths over.
            for(j=i+1; j<block->path_count; j++) {
                block->paths[j-1] = block->paths[j];
            }

            // Reallocate memory.
            block->path_count--;
            block->paths = realloc(block->paths, sizeof(sky_path*) * block->path_count);
            check_mem(block->paths);
            
            return 0;
        }
    }
    
    return 0;

error:
    return -1;
}