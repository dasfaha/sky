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
#include "path.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

/*
 * Creates a reference to a path.
 */
Path *Path_create()
{
    Path *path;
    
    path = malloc(sizeof(Path)); check_mem(path);
    path->events = NULL;
    path->event_count = 0;

    return path;
    
error:
    Path_destroy(path);
    return NULL;
}

/*
 * Removes a path reference from memory.
 */
void Path_destroy(Path *path)
{
    if(path) {
        // Destroy paths.
        if(path->event_count > 0) {
            uint32_t i=0;
            for(i=0; i<path->event_count; i++) {
                // TODO: Free event struct members only. Events will be freed at the end of loop.
                // Event_destroy(path->events[i]);
            }
        }
        
        if(path->events) free(path->events);
        path->events = NULL;
        path->event_count = 0;

        free(path);
    }
}


//======================================
// Event Management
//======================================

int Path_add_event(Path *path, Event *event)
{
    // TODO: Validate arguments.
    // TODO: Append event and resort events.

    return 0;
}
