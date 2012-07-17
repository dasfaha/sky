#include <stdlib.h>

#include "path_iterator.h"
#include "block.h"
#include "path.h"
#include "mem.h"
#include "dbg.h"


//==============================================================================
//
// Forward Declaration
//
//==============================================================================

int sky_path_iterator_get_ptr(sky_path_iterator *iterator, void **ptr);

int sky_path_iterator_get_current_block(sky_path_iterator *iterator,
    sky_block **block);

int sky_path_iterator_fast_forward(sky_path_iterator *iterator);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to a path iterator.
// 
// Returns a reference to the new path iterator if successful. Otherwise returns
// null.
sky_path_iterator *sky_path_iterator_create()
{
    sky_path_iterator *iterator = calloc(sizeof(sky_path_iterator), 1);
    check_mem(iterator);
    return iterator;
    
error:
    sky_path_iterator_free(iterator);
    return NULL;
}

// Removes a path iterator reference from memory.
//
// iterator - The path iterator to free.
void sky_path_iterator_free(sky_path_iterator *iterator)
{
    if(iterator) {
        iterator->data_file = NULL;
        free(iterator);
    }
}


//======================================
// Source
//======================================

// Assigns a data file as the source.
// 
// iterator  - The iterator.
// data_file - The data file to iterate over.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_set_data_file(sky_path_iterator *iterator, sky_data_file *data_file)
{
    int rc;
    check(iterator != NULL, "Iterator required");
    iterator->data_file   = data_file;
    iterator->block_index = 0;
    iterator->block       = NULL;
    iterator->byte_index  = 0;

    // Position iterator at the first path.
    rc = sky_path_iterator_fast_forward(iterator);
    check(rc == 0, "Unable to find next available path");

    return 0;
    
error:
    return -1;
}

// Assigns a block as the source.
// 
// iterator  - The iterator.
// block     - The block to iterate over.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_set_block(sky_path_iterator *iterator, sky_block *block)
{
    int rc;
    check(iterator != NULL, "Iterator required");
    iterator->block       = block;
    iterator->data_file   = NULL;
    iterator->block_index = 0;
    iterator->byte_index  = 0;

    // Position iterator at the first path.
    rc = sky_path_iterator_fast_forward(iterator);
    check(rc == 0, "Unable to find next available path");

    return 0;
    
error:
    return -1;
}


//======================================
// Block Management
//======================================

// Retrieves the current block that is being searched.
// 
// iterator - The iterator.
// block    - A pointer to where the current block is returned to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_get_current_block(sky_path_iterator *iterator,
                                        sky_block **block)
{
    check(iterator != NULL, "Iterator required");
    check(block != NULL, "Block return address required");
    
    // If we are iterating over a data file then return the current block.
    if(iterator->data_file != NULL) {
        *block = iterator->data_file->blocks[iterator->block_index];
    }
    // If we are iterating over a single block then just return that block.
    else {
        *block = iterator->block;
    }
    
    return 0;
    
error:
    return -1;
}

// Calculates the pointer address for a path that the iterator is currently
// pointing to.
//
// iterator - The iterator to calculate the address from.
//
// Returns a pointer to the address of the current path.
int sky_path_iterator_get_ptr(sky_path_iterator *iterator, void **ptr)
{
    int rc;

    // Retrieve the current block.
    sky_block *block;
    rc = sky_path_iterator_get_current_block(iterator, &block);
    check(rc == 0, "Unable to retrieve current block");
    
    // Retrieve the block pointer.
    rc = sky_block_get_ptr(block, ptr);
    check(rc == 0, "Unable to retrieve block pointer");

    // Increment by the byte offset.
    *ptr += iterator->byte_index;

    return 0;

error:
    *ptr = NULL;
    return -1;
}


//======================================
// Iteration
//======================================

// Moves the iterator to point to the next path.
// 
// iterator - The iterator.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_next(sky_path_iterator *iterator)
{
    int rc;
    check(iterator != NULL, "Iterator required");
    check(iterator->data_file != NULL || iterator->block != NULL, "Iterator must have a source");
    check(!iterator->eof, "Iterator is at end-of-file");

    // Retrieve some data file info.
    sky_data_file *data_file = (iterator->data_file ? iterator->data_file : iterator->block->data_file);
    
    // Retrieve current block.
    sky_block *block;
    rc = sky_path_iterator_get_current_block(iterator, &block);
    check(rc == 0, "Unable to retrieve current block");

    // If we are searching the data file and the current block is spanned then
    // move to the next block after the span.
    if(iterator->data_file && block->spanned) {
        // Retrieve span count.
        uint32_t span_count;
        rc = sky_block_get_span_count(block, &span_count);
        check(rc == 0, "Unable to calculate span count");
        
        // Move to the first block after the span.
        iterator->block_index += span_count;
        iterator->byte_index = 0;
    }
    // Otherwise move to the next path.
    else {
        // Find current pointer.
        void *ptr;
        rc = sky_path_iterator_get_ptr(iterator, &ptr);
        check(rc == 0, "Unable to retrieve the current pointer");

        // Read path size and move past it.
        iterator->byte_index += sky_path_sizeof_raw(ptr);
        
        // If the byte index is past the block size then move to next block.
        if(iterator->byte_index >= data_file->block_size) {
            iterator->block_index++;
            iterator->byte_index = 0;
        }
    }
    
    // Move to the next available path or mark the iterator as eof.
    rc = sky_path_iterator_fast_forward(iterator);
    check(rc == 0, "Unable to find next available path");
    
    return 0;
    
error:
    return -1;
}


// Moves the iterator to the next available path if it is not currently on a
// valid path. This can occur when the current location has null data or if
// an empty block is traversed over. If the iterator is at the end of the
// available paths then it is flagged as EOF.
// 
// iterator - The iterator.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_fast_forward(sky_path_iterator *iterator)
{
    int rc;
    check(iterator, "Iterator required");
    
    sky_data_file *data_file = iterator->data_file;
    
    // Keep searching for data or the EOF until we find it.
    while(true) {
        // If the block index is out of range then mark as EOF and exit.
        uint32_t max_block_index = (data_file != NULL ? data_file->block_count-1 : 0);
        if(iterator->block_index > max_block_index) {
            iterator->block_index = 0;
            iterator->byte_index  = 0;
            iterator->eof = true;
            break;
        }
        
        // If there is null data then move to the next block.
        void *ptr;
        rc = sky_path_iterator_get_ptr(iterator, &ptr);
        check(rc == 0, "Unable to retrieve the current pointer");
        
        // If there is null data then move to the next block.
        if(*((uint8_t*)ptr) == 0) {
            iterator->block_index++;
            iterator->byte_index = 0;
        }
        // If there is valid data then exit.
        else {
            // Grab the current object id.
            iterator->current_object_id = *((sky_object_id_t*)ptr);
            break;
        }
    }
    
    return 0;
    
error:
    return -1;
}
