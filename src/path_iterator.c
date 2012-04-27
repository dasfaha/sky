#include <stdlib.h>

#include "path_iterator.h"
#include "object_file.h"
#include "block.h"
#include "path.h"
#include "mem.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Utility
//======================================

// Calculates the pointer address for a path that the iterator is currently
// pointing to.
//
// iterator - The iterator to calculate the address from.
//
// Returns a pointer to the address of the current path.
void *get_data_address(PathIterator *iterator)
{
    BlockInfo *info = iterator->object_file->infos[iterator->block_index];
    return iterator->object_file->data + (info->id * iterator->object_file->block_size) + iterator->byte_index;
}

//======================================
// Lifecycle
//======================================

// Creates a reference to a path iterator.
// 
// object_file - The object file to iterate over.
//
// Returns a reference to the new path iterator if successful. Otherwise returns
// null.
PathIterator *PathIterator_create(ObjectFile *object_file)
{
    PathIterator *iterator;
    
    iterator = malloc(sizeof(PathIterator)); check_mem(iterator);
    iterator->object_file = object_file;
    iterator->block_index = 0;
    iterator->byte_index = BLOCK_HEADER_LENGTH;
    iterator->eof = false;

    return iterator;
    
error:
    PathIterator_destroy(iterator);
    return NULL;
}

// Removes a path iterator reference from memory.
//
// iterator - The path iterator to free.
void PathIterator_destroy(PathIterator *iterator)
{
    if(iterator) {
        iterator->object_file = NULL;
        free(iterator);
    }
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
int PathIterator_next(PathIterator *iterator, Cursor *cursor)
{
    int rc;
    int64_t zero = 0;
    
    check(iterator != NULL, "Iterator required");
    check(!iterator->eof, "Iterator is at end-of-file");
    check(cursor != NULL, "Existing cursor is required for iteration");

    // Retrieve some data file info.
    uint32_t block_size  = iterator->object_file->block_size;
    uint32_t block_count = iterator->object_file->block_count;
    
    // Loop until we find the next available path.
    void *ptr = NULL;
    while(true) {
        // Determine address of current location.
        BlockInfo *info = iterator->object_file->infos[iterator->block_index];
        ptr = get_data_address(iterator);

        // If byte index is too close to the end-of-block or if there is no more
        // data in the block then go to the next block.
        if(iterator->byte_index >= block_size-PATH_HEADER_LENGTH ||
           memcmp(ptr, &zero, sizeof(zero)) == 0)
        {
            // Move to the next block.
            iterator->block_index += 1;
            iterator->byte_index = BLOCK_HEADER_LENGTH;

            // If we have reached the end of the data file then NULL out
            // the pointer and change the iterator state to EOF.
            if(iterator->block_index >= block_count) {
                iterator->block_index = 0;
                iterator->eof = true;
                rc = Cursor_set_path(cursor, NULL);
                check(rc == 0, "Unable to clear cursor path");
                break;
            }
        }
        // If a path is found then exit the loop and update iterator state to
        // the next path.
        else {
            // If this is a spanned block then move to the next block after the
            // span.
            if(info->spanned) {
                uint32_t i, span_count;

                rc = ObjectFile_get_block_span_count(iterator->object_file, iterator->block_index, &span_count);
                check(rc == 0, "Unable to calculate span count");
                iterator->byte_index = BLOCK_HEADER_LENGTH;

                // Collect all paths across spanned blocks.
                void **paths = malloc(sizeof(void*) * span_count); check_mem(paths);
                for(i=0; i<span_count; i++) {
                    paths[i] = get_data_address(iterator);
                    iterator->block_index++;
                }
                rc = Cursor_set_paths(cursor, paths, span_count);
                check(rc == 0, "Unable to set paths on multi-block cursor");
            }
            // Otherwise move the byte index ahead to the next path.
            else {
                rc = Cursor_set_path(cursor, ptr);
                check(rc == 0, "Unable to set path on cursor");
                iterator->byte_index += Path_get_length(ptr);
            }
            break;
        }
    }
    
    return 0;
    
error:
    return -1;
}
