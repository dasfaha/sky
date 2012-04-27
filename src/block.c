#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include "dbg.h"
#include "block.h"
#include "mem.h"

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
    Path **a = (Path**)_a;
    Path **b = (Path**)_b;

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
void sort_paths(Block *block)
{
    qsort(block->paths, block->path_count, sizeof(Path*), compare_paths);
}


//======================================
// Lifecycle
//======================================

// Creates a reference to an in-memory block.
//
// object_file - The object file that this block belongs to.
// info        - The header information about block.
//
// Returns a new block if successful, otherwise returns null.
Block *Block_create(ObjectFile *object_file, BlockInfo *info)
{
    Block *block;
    
    block = malloc(sizeof(Block)); check_mem(block);

    block->object_file = object_file;
    block->info = info;

    block->paths = NULL;
    block->path_count = 0;

    return block;
    
error:
    Block_destroy(block);
    return NULL;
}

// Removes a block reference from memory.
//
// block - The block to free.
void Block_destroy(Block *block)
{
    if(block) {
        block->object_file = NULL;
        block->info = NULL;

        // Destroy paths.
        uint32_t i=0;
        for(i=0; i<block->path_count; i++) {
            Path_destroy(block->paths[i]);
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
uint32_t get_paths_length(Block *block)
{
    uint32_t i;
    uint32_t length = 0;
    
    // Add size for each path.
    for(i=0; i<block->path_count; i++) {
        length += Path_get_serialized_length(block->paths[i]);
    }
    
    return length;
}

// Calculates the total number of bytes needed to store a block and its paths.
// This number does not include the padding added after the block.
//
// block - The block.
uint32_t Block_get_serialized_length(Block *block)
{
    uint32_t length = 0;

    // Add path count and path length.
    length += BLOCK_HEADER_LENGTH;
    length += get_paths_length(block);
    
    return length;
}

// Serializes a block at a given memory location.
//
// block  - The block to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int Block_serialize(Block *block, void *addr, ptrdiff_t *length)
{
    int rc;
    void *start = addr;

    // Validate.
    check(block != NULL, "Block required");
    check(addr != NULL, "Address required");

    // Write path count.
    memwrite(addr, &block->path_count, sizeof(block->path_count), "block path count");
    
    // Loop over paths and delegate serialization to each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        ptrdiff_t ptrdiff;
        rc = Path_serialize(block->paths[i], addr, &ptrdiff);
        check(rc == 0, "Unable to serialize block path: %d", i);
        addr += ptrdiff;
    }
    
    // Null fill the rest of the block.
    int fillcount = block->object_file->block_size - (addr-start);
    check(memset(addr, 0, fillcount) != NULL, "Unable to null fill end of block");
    
    // Store number of bytes written.
    if(length != NULL) {
        *length = (addr-start);
    }
    
    return 0;

error:
    *length = 0;
    return -1;
}

// Deserializes a block from a given memory location.
//
// block - The block to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int Block_deserialize(Block *block, void *addr, ptrdiff_t *length)
{
    int rc;
    void *start = addr;

    // Validate.
    check(block != NULL, "Block required");
    check(addr != NULL, "Address required");

    // Read path count.
    memread(addr, &block->path_count, sizeof(block->path_count), "block path count");

    // Allocate paths.
    block->paths = realloc(block->paths, sizeof(Path*) * block->path_count);
    check_mem(block->paths);

    // Loop over paths and delegate deserialization to each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        ptrdiff_t ptrdiff;
        Path *path = Path_create(0); check_mem(path);
        block->paths[i] = path;
        
        rc = Path_deserialize(path, addr, &ptrdiff);
        check(rc == 0, "Unable to deserialize block path: %d", i);
        addr += ptrdiff;
    }

    // Store number of bytes read.
    if(length != NULL) {
        *length = (addr-start);
    }

    return 0;

error:
    *length = 0;
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
int Block_update_info(Block *block)
{
    int64_t min_object_id = 0;
    int64_t max_object_id = 0;
    int64_t min_timestamp = INT64_MIN;
    int64_t max_timestamp = INT64_MIN;
    
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
            Path *path = block->paths[i];
        
            // Find object id range.
            if(min_object_id == 0 || path->object_id < min_object_id) {
                min_object_id = path->object_id;
            }
            if(max_object_id == 0 || path->object_id > max_object_id) {
                max_object_id = path->object_id;
            }
        
            // Iterate over events and find timstamp range.
            for(j=0; j<path->event_count; j++) {
                Event *event = path->events[j];
            
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
    BlockInfo *info = block->info;
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
int Block_add_event(Block *block, Event *event)
{
    uint32_t i;
    int rc;
    
    // Validation.
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");
    check(event->object_id != 0, "Event object id cannot be 0");

    // Find existing path by object id.
    Path *path = NULL;
    for(i=0; i<block->path_count; i++) {
        if(block->paths[i]->object_id == event->object_id) {
            path = block->paths[i];
            break;
        }
    }
    
    // If matching path could not be found then create one.
    if(path == NULL) {
        path = Path_create(event->object_id); check_mem(path);
        rc = Block_add_path(block, path);
        check(rc == 0, "Unable to add new path to block")
    }
    
    // Add event to path.
    rc = Path_add_event(path, event);
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
int Block_remove_event(Block *block, Event *event)
{
    uint32_t i, j, index;
    int rc;
    
    // Validation.
    check(block != NULL, "Block required");
    check(event != NULL, "Event required");
    check(event->object_id != 0, "Event object id cannot be 0");

    // Find event in paths.
    Path *path;

    for(i=0; i<block->path_count; i++) {
        bool found = false;
        path = block->paths[i];
        
        // If object id matches then search path.
        if(path->object_id == event->object_id) {
            for(j=0; j<path->event_count; j++) {
                // If event is found then remove it.
                if(path->events[j] == event) {
                    rc = Path_remove_event(path, event);
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
        Block_remove_path(block, path);
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
int Block_add_path(Block *block, Path *path)
{
    // Validation.
    check(block != NULL, "Block required");
    check(path != NULL, "Path required");
    check(path->object_id != 0, "Path object id cannot be 0");

    // Allocate space for path.
    block->path_count++;
    block->paths = realloc(block->paths, sizeof(Event*) * block->path_count);
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
int Block_remove_path(Block *block, Path *path)
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
            block->paths = realloc(block->paths, sizeof(Path*) * block->path_count);
            check_mem(block->paths);
            
            return 0;
        }
    }
    
    return 0;

error:
    return -1;
}