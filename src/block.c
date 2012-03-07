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

/*
 * Creates a reference to a block.
 */
Block *Block_create()
{
    Block *block;
    
    block = malloc(sizeof(Block)); check_mem(block);
    block->paths = NULL;
    block->path_count = 0;

    return block;
    
error:
    Block_destroy(block);
    return NULL;
}

/*
 * Removes a block reference from memory.
 */
void Block_destroy(Block *block)
{
    if(block) {
        // Destroy paths.
        if(block->path_count > 0) {
            uint32_t i=0;
            for(i=0; i<block->path_count; i++) {
                // TODO: Release path struct members. Releasing full path list at the end of this loop.
                // Path_destroy(block->paths[i]);
            }
        }
        
        if(block->paths) free(block->paths);
        block->paths = NULL;
        block->path_count = 0;

        free(block);
    }
}


//======================================
// Event Management
//======================================

int Block_add_event(Block *block, Event *event)
{
    // TODO: Validate arguments.
    // TODO: Find path in block with matching event's object id.
    // TODO: If none found then append a new path.
    // TODO: Add event to path.

    return 0;
}
