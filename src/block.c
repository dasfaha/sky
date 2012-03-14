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
#include <inttypes.h>
#include <unistd.h>

#include "dbg.h"
#include "block.h"

//==============================================================================
//
// Functions
//
//==============================================================================

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

// Serializes a block to a given file at the file's current offset.
//
// block - The block to serialize.
// fd    - The file descriptor.
//
// Returns 0 if successful, otherwise returns -1.
int Block_serialize(Block *block, int fd)
{
    int rc;

    // Validate.
    check(block != NULL, "Block required");
    check(fd != -1, "File descriptor required");
    
    // Write path count.
    rc = write(fd, &block->path_count, sizeof(block->path_count));
    check(rc == sizeof(block->path_count), "Unable to serialize block path count: %d", block->path_count);
    
    // Loop over paths and delegate serialization to each path.
    uint32_t i;
    for(i=0; i<block->path_count; i++) {
        rc = Path_serialize(block->paths[i], fd);
        check(rc == 0, "Unable to serialize block path: %d", i);
    }
    
    return 0;

error:
    return -1;
}

// Deserializes a block from a given file at the file's current offset.
//
// block - The block to serialize.
// fd    - The file descriptor.
//
// Returns 0 if successful, otherwise returns -1.
int Block_deserialize(Block *block, int fd)
{
    // TODO: Read path count.
    // TODO: Loop over paths and delegate serialization to each path.

    return 0;
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
    // TODO: Validate arguments.
    // TODO: Find path in block with matching event's object id.
    // TODO: If none found then append a new path.
    // TODO: Add event to path.

    return 0;
}
