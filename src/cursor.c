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

#include "cursor.h"
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

// Creates a reference to a cursor.
// 
// Returns a reference to the new cursor if successful. Otherwise returns
// null.
Cursor *Cursor_create()
{
    Cursor *cursor;
    
    cursor = malloc(sizeof(Cursor)); check_mem(cursor);
    cursor->paths = NULL;
    cursor->path_count = 0;

    return cursor;
    
error:
    Cursor_destroy(cursor);
    return NULL;
}

// Removes a cursor reference from memory.
//
// cursor - The cursor to free.
void Cursor_destroy(Cursor *cursor)
{
    if(cursor) {
        if(cursor->paths) free(cursor->paths);
        free(cursor);
    }
}


//======================================
// Path Management
//======================================

// Assigns a single path to the cursor.
// 
// cursor - The cursor.
// ptr    - A pointer to the raw data where the path starts.
//
// Returns 0 if successful, otherwise returns -1.
int Cursor_set_path(Cursor *cursor, void *ptr)
{
    int rc;
    
    check(cursor != NULL, "Cursor required");

    // If data is not null then create an array of one pointer.
    if(ptr != NULL) {
        void **ptrs = malloc(sizeof(void*));
        ptrs[0] = ptr;
        rc = Cursor_set_paths(cursor, ptrs, 1);
        check(rc == 0, "Unable to set path data to cursor");
    }
    else {
        rc = Cursor_set_paths(cursor, NULL, 0);
        check(rc == 0, "Unable to remove path data");
    }

    return 0;

error:
    return -1;
}

// Assigns a list of paths to the cursor.
// 
// cursor - The cursor.
// ptrs   - An array to pointers of raw paths.
//
// Returns 0 if successful, otherwise returns -1.
int Cursor_set_paths(Cursor *cursor, void **ptrs, int count)
{
    check(cursor != NULL, "Cursor required");
    
    // Free old path list.
    if(cursor->paths != NULL) {
        free(cursor->paths);
    }

    // Assign path data list.
    cursor->paths = ptrs;
    cursor->path_count = count;
    
    return 0;

error:
    return -1;
}
