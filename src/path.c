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
Path *Path_create(int64_t object_id)
{
    Path *path;
    
    path = malloc(sizeof(Path)); check_mem(path);
    
    path->object_id = object_id;
    
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
        // Destroy events.
        uint32_t i=0;
        for(i=0; i<path->event_count; i++) {
            Event_destroy(path->events[i]);
        }
        
        if(path->events) free(path->events);
        path->events = NULL;
        path->event_count = 0;

        free(path);
    }
}


//======================================
// Serialization
//======================================

// Calculates the total number of bytes needed to store just the events section
// of the path.
uint32_t get_events_length(Path *path)
{
    uint32_t i;
    uint32_t length = 0;
    
    // Add size for each event.
    for(i=0; i<path->event_count; i++) {
        length += Event_get_serialized_length(path->events[i]);
    }
    
    return length;
}

// Calculates the total number of bytes needed to store a path and its events.
//
// path - The path.
uint32_t Path_get_serialized_length(Path *path)
{
    uint32_t length = 0;

    // Add object id.
    length += sizeof(path->object_id);

    // Add event length.
    uint32_t events_length = get_events_length(path);
    length += sizeof(events_length);
    length += events_length;
    
    return length;
}

// Serializes a path to a given file at the file's current offset.
//
// path - The path to serialize.
// fd   - The file descriptor.
//
// Returns 0 if successful, otherwise returns -1.
int Path_serialize(Path *path, int fd)
{
    uint32_t i;
    int rc;
    
    // Validate.
    check(path != NULL, "Block required");
    check(fd != -1, "File descriptor required");

    // Write object id.
    rc = write(fd, &path->object_id, sizeof(path->object_id));
    check(rc == sizeof(path->object_id), "Unable to serialize path object id: %lld", path->object_id);
    
    // Write events length.
    uint32_t events_length = get_events_length(path);
    rc = write(fd, &events_length, sizeof(events_length));
    check(rc == sizeof(events_length), "Unable to serialize path events length: %d", events_length);

    // Serialize events.
    for(i=0; i<path->event_count; i++) {
        rc = Event_serialize(path->events[i], fd);
        check(rc == 0, "Unable to serialize path event: %d", i);
    }

    return 0;

error:
    return -1;
}

// Deserializes a path from a given file at the file's current offset.
//
// path - The path to serialize.
// fd   - The file descriptor.
//
// Returns 0 if successful, otherwise returns -1.
int Path_deserialize(Path *path, int fd)
{
    // TODO: Read path count.
    // TODO: Loop over paths and delegate serialization to each path.

    return 0;
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
