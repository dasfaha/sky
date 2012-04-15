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
// Event Sorting
//======================================

// Compares two events and sorts them by timestamp and then orders data events
// before action events.
int compare_events(const void *_a, const void *_b)
{
    Event **a = (Event**)_a;
    Event **b = (Event**)_b;

    // Sort by timestamp first.
    if((*a)->timestamp > (*b)->timestamp) {
        return 1;
    }
    else if((*a)->timestamp < (*b)->timestamp) {
        return -1;
    }
    else {
        // Place data items first.
        int ad = ((*a)->data_count > 0);
        int bd = ((*b)->data_count > 0);

        if(ad > bd) {
            return 1;
        }
        else if(ad < bd) {
            return -1;
        }
        else {
            return 0;
        }
    }
}

// Sorts events in a path.
//
// path - The path containing the events.
void sort_events(Path *path)
{
    qsort(path->events, path->event_count, sizeof(Event*), compare_events);
}


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

    // Add header length
    length += PATH_HEADER_LENGTH;

    // Add event length.
    uint32_t events_length = get_events_length(path);
    length += events_length;
    
    return length;
}

// Serializes a path to a given file at the file's current offset.
//
// path - The path to serialize.
// fd   - The file descriptor.
//
// Returns 0 if successful, otherwise returns -1.
int Path_serialize(Path *path, FILE *file)
{
    uint32_t i;
    int rc;
    
    // Validate.
    check(path != NULL, "Path required");
    check(file != NULL, "File descriptor required");

    // Write object id.
    rc = fwrite(&path->object_id, sizeof(path->object_id), 1, file);
    check(rc == 1, "Unable to serialize path object id: %lld", path->object_id);
    
    // Write events length.
    uint32_t events_length = get_events_length(path);
    rc = fwrite(&events_length, sizeof(events_length), 1, file);
    check(rc == 1, "Unable to serialize path events length: %d", events_length);

    // Serialize events.
    for(i=0; i<path->event_count; i++) {
        rc = Event_serialize(path->events[i], file);
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
int Path_deserialize(Path *path, FILE *file)
{
    int rc;

    // Validate.
    check(path != NULL, "Path required");
    check(file != NULL, "File descriptor required");

    // Read object id.
    rc = fread(&path->object_id, sizeof(path->object_id), 1, file);
    check(rc == 1, "Unable to deserialize path object id");

    // Read events length.
    uint32_t events_length;
    rc = fread(&events_length, sizeof(events_length), 1, file);
    check(rc == 1, "Unable to deserialize path events length: %d", events_length);

    // Deserialize events.
    int index = 0;
    long endpos = ftell(file) + events_length;
    while(!feof(file) && ftell(file) < endpos) {
        path->event_count++;
        path->events = realloc(path->events, sizeof(Event*) * path->event_count);
        check_mem(path->events);

        path->events[index] = Event_create(0, path->object_id, 0);
        rc = Event_deserialize(path->events[index], file);
        check(rc == 0, "Unable to deserialize event: %d", index);
        index++;
    }

    return 0;

error:
    return -1;
}


//======================================
// Event Management
//======================================

// Adds an event to a path. An event can only be added if the event's object id
// matches the path's object id.
//
// path  - The path to add the event to.
// event - The event to add to the path.
//
// Returns 0 if successful, otherwise returns -1.
int Path_add_event(Path *path, Event *event)
{
    // Validation.
    check(path != NULL, "Path required");
    check(path->object_id != 0, "Path object id cannot be null");
    check(event != NULL, "Event required");
    check(path->object_id == event->object_id, "Event object id (%lld) does not match path object id (%lld)", event->object_id, path->object_id);

    // Raise error if event has already been added.
    unsigned int i;
    for(i=0; i<path->event_count; i++) {
        if(path->events[i] == event) {
            sentinel("Event has already been added to path");
        }
    }

    // Allocate space for event.
    path->event_count++;
    path->events = realloc(path->events, sizeof(Event*) * path->event_count);
    check_mem(path->events);

    // Append event to the end.
    path->events[path->event_count-1] = event;

    // Sort events.
    sort_events(path);

    return 0;

error:
    return -1;
}

// Removes an event from a path.
//
// path  - The path that contains the event.
// event - The event to remove.
//
// Returns 0 if successful, otherwise returns -1.
int Path_remove_event(Path *path, Event *event)
{
    // Validation.
    check(path != NULL, "Path required");
    check(event != NULL, "Event required");

    // Find event.
    unsigned int i, j;
    for(i=0; i<path->event_count; i++) {
        if(path->events[i] == event) {
            // Shift events over.
            for(j=i+1; j<path->event_count; j++) {
                path->events[j-1] = path->events[j];
            }
            
            // Reallocate memory.
            path->event_count--;
            path->events = realloc(path->events, sizeof(Event*) * path->event_count);
            check_mem(path->events);
            
            break;
        }
    }

    // Sort events.
    sort_events(path);

    return 0;

error:
    return -1;
}
