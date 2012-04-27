#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include "dbg.h"
#include "path.h"
#include "mem.h"

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

// Calculates the full length of a path at a given pointer address.
// 
// ptr - A pointer to raw, serialized path data.
//
// Returns the length of the path.
uint32_t Path_get_length(const void *ptr)
{
    // Read events length from raw data.
    uint32_t events_length;
    memcpy(&events_length, ptr+sizeof(int64_t), sizeof(events_length));
    
    return PATH_HEADER_LENGTH + events_length;
}

// Serializes a path at a given memory location.
//
// path   - The path to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int Path_serialize(Path *path, void *addr, ptrdiff_t *length)
{
    uint32_t i;
    int rc;
    void *start = addr;
    
    // Validate.
    check(path != NULL, "Path required");
    check(addr != NULL, "Address required");

    // Write object id.
    memwrite(addr, &path->object_id, sizeof(path->object_id), "path object id");
    
    // Write events length.
    uint32_t events_length = get_events_length(path);
    memwrite(addr, &events_length, sizeof(events_length), "path events length");

    // Serialize events.
    for(i=0; i<path->event_count; i++) {
        ptrdiff_t ptrdiff;
        rc = Event_serialize(path->events[i], addr, &ptrdiff);
        check(rc == 0, "Unable to serialize path event: %d", i);
        addr += ptrdiff;
    }

    // Store number of bytes written.
    if(length != NULL) {
        *length = (addr-start);
    }

    return 0;

error:
    *length = 0;
    return -1;
}

// Deserializes a path to a memory location.
//
// path   - The path to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int Path_deserialize(Path *path, void *addr, ptrdiff_t *length)
{
    int rc;
    void *start = addr;

    // Validate.
    check(path != NULL, "Path required");
    check(addr != NULL, "Address required");

    // Read object id.
    memread(addr, &path->object_id, sizeof(path->object_id), "path object id");

    // Read events length.
    uint32_t events_length;
    memread(addr, &events_length, sizeof(events_length), "path events length");

    // Deserialize events.
    int index = 0;
    void *endptr = addr + events_length;
    while(addr < endptr) {
        ptrdiff_t ptrdiff;
        
        path->event_count++;
        path->events = realloc(path->events, sizeof(Event*) * path->event_count);
        check_mem(path->events);

        path->events[index] = Event_create(0, path->object_id, 0);
        rc = Event_deserialize(path->events[index], addr, &ptrdiff);
        check(rc == 0, "Unable to deserialize event: %d", index);
        addr += ptrdiff;
        
        index++;
    }

    // Store number of bytes read.
    if(length != NULL) {
        *length = (addr-start);
    }

    return 0;

error:
    *length = 0;
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
