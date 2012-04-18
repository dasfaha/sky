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
#include <stdbool.h>
#include <unistd.h>

#include "dbg.h"
#include "bstring.h"
#include "event.h"
#include "mem.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Utility
//======================================

// Determines whether the event has an action set on it.
bool has_action(Event *event)
{
    return (event->action_id != 0);
}

// Determines whether the event has a data set on it.
bool has_data(Event *event)
{
    return (event->data_count > 0);
}

// Calculates the flag byte for the event based on whether the event has an
// action and whether the event has data.
uint8_t get_event_flag(Event *event)
{
    uint8_t flag = 0;

    if(has_action(event)) {
        flag |= EVENT_FLAG_ACTION;
    }
    if(has_data(event)) {
        flag |= EVENT_FLAG_DATA;
    }

    return flag;
}


//======================================
// Data Allocation
//======================================

// Deallocates all event data items for an event.
//
// event - The event to clear data for.
//
// Return 0 if successful, otherwise returns -1.
void clear_data(Event *event)
{
    event->data_count = 0;
    if(event->data) free(event->data);
    event->data = NULL;
}

// Allocates an additional event data item for an event.
//
// event - The event to allocate data for.
//
// Return 0 if successful, otherwise returns -1.
int allocate_data(Event *event)
{
    event->data_count++;
    event->data = realloc(event->data, sizeof(EventData*) * event->data_count);
    check_mem(event->data);
    return 0;
    
error:
    return -1;
}

// Deallocates a single event data item for an event.
//
// event - The event to deallocate data for.
//
// Return 0 if successful, otherwise returns -1.
int deallocate_data(Event *event)
{
    // Raise error if no data left.
    if(event->data_count == 0) {
        sentinel("Requested to deallocate more event data than available");
    }
    // Completely clear data if data count is 0.
    else if(event->data_count == 1) {
        clear_data(event);
    }
    // Otherwise deallocate items.
    else {
        event->data_count--;
        event->data = realloc(event->data, sizeof(EventData*) * event->data_count);
        check_mem(event->data);
    }

    return 0;
    
error:
    return -1;
}


//======================================
// Lifecycle
//======================================

// Creates a reference to an event.
//
// timestamp - When the event occurred (in milliseconds since midnight Jan 1,
//             1970 UTC).
// object_id - The identifier for the object that the event is related to.
// action    - The name of the action that was performed.
Event *Event_create(int64_t timestamp, int64_t object_id, int32_t action_id)
{
    Event *event;
    
    event = malloc(sizeof(Event)); check_mem(event);
    
    event->timestamp = timestamp;
    event->object_id = object_id;
    event->action_id = action_id;

    event->data = NULL;
    event->data_count = 0;

    return event;
    
error:
    Event_destroy(event);
    return NULL;
}

// Removes an event reference from memory.
void Event_destroy(Event *event)
{
    if(event) {
        // Destroy data.
        uint32_t i=0;
        for(i=0; i<event->data_count; i++) {
            EventData_destroy(event->data[i]);
        }
        
        if(event->data) free(event->data);
        event->data = NULL;
        event->data_count = 0;

        free(event);
    }
}

// Creates a copy of an event.
//
// source - The event to copy.
// target - A reference to the new event object returned to the caller.
//
// Returns 0 if successful, otherwise returns -1.
int Event_copy(Event *source, Event **target)
{
    int rc;
    EventData *data;

    check(source != NULL, "Source event is required for copy");

    // Copy basic properties.
    Event *event = Event_create(source->timestamp, source->object_id, source->action_id);
    
    // Copy event data.
    if(source->data_count > 0) {
        // Allocate memory for data.
        event->data_count = source->data_count;
        event->data = malloc(sizeof(EventData*) * event->data_count);
        check_mem(event->data);

        // Copy each event data item.
        int i;
        for(i=0; i<event->data_count; i++) {
            rc = EventData_copy(source->data[i], &data);
            check(rc == 0, "Unable to copy event data");
            event->data[i] = data;
        }
    }

    // Return event to the caller.
    *target = event;
    
    return 0;
    
error:
    if(event) Event_destroy(event);
    *target = NULL;

    return -1;
}



//======================================
// Serialization
//======================================

// Calculates the total number of bytes needed to store just the data section
// of the event.
uint16_t get_data_length(Event *event)
{
    uint16_t i;
    uint16_t length = 0;
    
    // Add size for each data item.
    for(i=0; i<event->data_count; i++) {
        length += EventData_get_serialized_length(event->data[i]);
    }
    
    return length;
}

// Calculates the total number of bytes needed to store an event.
//
// event - The event.
uint32_t Event_get_serialized_length(Event *event)
{
    uint32_t length = 0;
    
    // Add event flag.
    length += 1;
    
    // Add timestamp.
    length += sizeof(event->timestamp);
    
    // Add action if one is set.
    if(has_action(event)) {
        length += sizeof(event->action_id);
    }
    
    // Add data if set.
    if(has_data(event)) {
        uint16_t data_length = get_data_length(event);

        length += sizeof(data_length);
        length += data_length;
    }

    return length;
}

// Serializes an event to memory at a given pointer location.
//
// event  - The event to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int Event_serialize(Event *event, void *addr, ptrdiff_t *length)
{
    int rc;
    void *start = addr;
    
    // Validate.
    check(event != NULL, "Event required");
    check(addr != NULL, "Address required");
    
    // Write event flag.
    uint8_t flag = get_event_flag(event);
    memwrite(addr, &flag, sizeof(flag), "event flag");
    
    // Write timestamp.
    memwrite(addr, &event->timestamp, sizeof(event->timestamp), "event timestamp");
    
    // Write action if set.
    if(has_action(event)) {
        memwrite(addr, &event->action_id, sizeof(event->action_id), "event action");
    }

    // Write data if set.
    if(has_data(event)) {
        // Write data length.
        uint16_t data_length = get_data_length(event);
        memwrite(addr, &data_length, sizeof(data_length), "event data length");
        
        // Serialize data.
        int i;
        for(i=0; i<event->data_count; i++) {
            ptrdiff_t ptrdiff;
            rc = EventData_serialize(event->data[i], addr, &ptrdiff);
            check(rc == 0, "Unable to serialize event data: %d", i);
            addr += ptrdiff;
        }
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

// Deserializes an event from a given file at the file's current offset.
//
// event  - The event to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int Event_deserialize(Event *event, void *addr, ptrdiff_t *length)
{
    int rc;
    void *start = addr;

    // Validate.
    check(event != NULL, "Event required");
    check(addr != NULL, "Address required");

    // Read event flag.
    uint8_t flag;
    memread(addr, &flag, sizeof(flag), "event flag");

    // Read timestamp.
    memread(addr, &event->timestamp, sizeof(event->timestamp), "event timestamp");

    // Read action if one exists.
    if(flag & EVENT_FLAG_ACTION) {
        memread(addr, &event->action_id, sizeof(event->action_id), "event action");
    }

    // Clear existing data.
    clear_data(event);
    
    // Read data if set.
    if(flag & EVENT_FLAG_DATA) {
        // Read data length.
        uint16_t data_length;
        memread(addr, &data_length, sizeof(data_length), "event data length");

        // Deserialize data.
        int index = 0;
        void *endptr = addr + data_length;
        while(addr < endptr) {
            ptrdiff_t ptrdiff;

            check(allocate_data(event) == 0, "Unable to append event data");
            event->data[index] = EventData_create(0, NULL);

            rc = EventData_deserialize(event->data[index], addr, &ptrdiff);
            check(rc == 0, "Unable to deserialize event data: %d", index);
            addr += ptrdiff;

            index++;
        }
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
// Event Data
//======================================

// Retrieves the data element for a given key on an event.
//
// event - The event containing the data.
// key   - The key to lookup.
// data  - A reference to the matching data if found. Otherwise this is set to
//         NULL.
//
// Returns 0 if successful, otherwise -1.
int Event_get_data(Event *event, int16_t key, EventData **data)
{
    int i;
    bool found = false;
    
    // Validation.
    check(event != NULL, "Event required");

    // Loop over data to find matching key.
    for(i=0; i<event->data_count; i++) {
        if(event->data[i]->key == key) {
            *data = event->data[i];
            found = true;
            break;
        }
    }
    
    // Return null if not found.
    if(!found) {
        *data = NULL;
    }
    
    return 0;

error:
    return -1;
}

// Sets data on an event.
//
// key - The key id to set for the data.
// value - The value to set on the data.
//
// Returns 0 if successful, otherwise returns -1.
int Event_set_data(Event *event, int16_t key, bstring value)
{
    int rc;
    EventData *data = NULL;
    
    // Validation.
    check(event != NULL, "Event required");
    
    // Find data with existing key.
    rc = Event_get_data(event, key, &data);
    check(rc == 0, "Unable to find event data")
    
    // If existing key is found then update value.
    if(data != NULL) {
        // Cleanup old value.
        if(data->value != NULL) {
            bdestroy(data->value);
        }
    
        // Assign new value.
        data->value = value;
    }
    // Otherwise append a new data item.
    else {
        check(allocate_data(event) == 0, "Unable to allocate event data");
        event->data[event->data_count-1] = EventData_create(key, value);
    }

    return 0;
    
error:
    return -1;
}

// Removes the value associated with a given key on the event data.
//
// event - The event that contains the data.
// key   - The key to unset.
//
// Returns 0 if successful, otherwise returns -1.
int Event_unset_data(Event *event, int16_t key)
{
    int i, j;
    
    // Validation.
    check(event != NULL, "Event required");

    // Loop over data to find matching key.
    for(i=0; i<event->data_count; i++) {
        // If found then unload item and resize array down.
        if(event->data[i]->key == key) {
            // Destroy data.
            EventData_destroy(event->data[i]);
            
            // Shift all items left.
            for(j=i+1; j<event->data_count; j++) {
                event->data[j-1] = event->data[j];
            }

            // Resize array.
            check(deallocate_data(event) == 0, "Unable to deallocate event data");

            break;
        }
    }
    
    return 0;

error:
    return -1;
}
