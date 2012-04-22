/*
 * Copyright (c) 2012 Ben Johnson, http://skylandlabs.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>

#include "path_iterator.h"
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
        ptr = iterator->object_file->data + (iterator->block_index * block_size) + iterator->byte_index;

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
            if(iterator->block_index > block_count) {
                iterator->block_index = 0;
                iterator->byte_index  = 0;
                iterator->eof = true;
                ptr = NULL;
                break;
            }
        }
        // If a path is found then exit the loop and update iterator state to
        // the next path.
        else {
            // Move byte index to the end of the path before exiting.
            iterator->byte_index += Path_get_length(ptr);
            break;
        }
    }
    
    // Point the cursor at the current location.
    cursor->ptr = ptr;
    
    // TODO: Change cursor to support multiple paths for spanned blocks.
    
    return 0;
    
error:
    return -1;
}
