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
// data_file - The data file to iterate over.
//
// Returns a reference to the new path iterator if successful. Otherwise returns
// null.
sky_path_iterator *sky_path_iterator_create(sky_data_file *data_file)
{
    sky_path_iterator *iterator;
    
    iterator = calloc(sizeof(sky_path_iterator), 1); check_mem(iterator);
    iterator->data_file = data_file;

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
// Pointer Management
//======================================

// Calculates the pointer address for a path that the iterator is currently
// pointing to.
//
// iterator - The iterator to calculate the address from.
//
// Returns a pointer to the address of the current path.
int sky_path_iterator_get_ptr(sky_path_iterator *iterator, void **ptr)
{
    // Retrieve the block pointer.
    sky_block *block = iterator->data_file->blocks[iterator->block_index];
    int rc = sky_block_get_ptr(block, ptr);
    check(rc == 0, "Unable to retrieve block pointer");

    // Increment by the byte offset.
    ptr += iterator->byte_index;

    return 0;

error:
    *ptr = NULL;
    return -1;
}


//======================================
// Iteration
//======================================

// Retrieves a cursor pointing to the next available path.
// 
// iterator - The iterator.
// cursor   - A reference to an existing cursor to use.
//
// Returns 0 if successful, otherwise returns -1.
int sky_path_iterator_next(sky_path_iterator *iterator, sky_cursor *cursor)
{
    int rc;
    
    check(iterator != NULL, "Iterator required");
    check(!iterator->eof, "Iterator is at end-of-file");
    check(cursor != NULL, "Existing cursor is required for iteration");

    // Retrieve some data file info.
    uint32_t block_size  = iterator->data_file->block_size;
    uint32_t block_count = iterator->data_file->block_count;
    
    // Loop until we find the next available path.
    void *ptr = NULL;
    while(true) {
        // Determine address of current location.
        rc = sky_path_iterator_get_ptr(iterator, &ptr);
        check(rc == 0, "Unable to retrieve the path pointer");

        // If byte index is too close to the end-of-block or if there is no more
        // data in the block then go to the next block.
        sky_block *block = iterator->data_file->blocks[iterator->block_index];
        if(iterator->byte_index >= block_size || *((uint8_t*)ptr) == 0)
        {
            // Move to the next block.
            iterator->block_index += 1;
            iterator->byte_index = 0;

            // If we have reached the end of the data file then NULL out
            // the pointer and change the iterator state to EOF.
            if(iterator->block_index >= block_count) {
                iterator->block_index = 0;
                iterator->eof = true;
                rc = sky_cursor_set_path(cursor, NULL);
                check(rc == 0, "Unable to clear cursor path");
                break;
            }
        }
        // If a path is found then exit the loop and update iterator state to
        // the next path.
        else {
            // If this is a spanned block then move to the next block after the
            // span.
            if(block->spanned) {
                uint32_t i, span_count;

                rc = sky_block_get_span_count(block, &span_count);
                check(rc == 0, "Unable to calculate span count");
                iterator->byte_index = 0;

                // Collect all paths across spanned blocks.
                void **paths = malloc(sizeof(void*) * span_count); check_mem(paths);
                for(i=0; i<span_count; i++) {
                    rc = sky_path_iterator_get_ptr(iterator, &paths[i]);
                    check(rc == 0, "Unable to retrieve the path (#%d) pointer", i);
                    iterator->block_index++;
                }
                rc = sky_cursor_set_paths(cursor, paths, span_count);
                check(rc == 0, "Unable to set paths on multi-block cursor");
            }
            // Otherwise move the byte index ahead to the next path.
            else {
                rc = sky_cursor_set_path(cursor, ptr);
                check(rc == 0, "Unable to set path on cursor");
                iterator->byte_index += sky_path_sizeof(ptr);
            }
            break;
        }
    }
    
    return 0;
    
error:
    return -1;
}
