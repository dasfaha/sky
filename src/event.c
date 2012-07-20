#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "dbg.h"
#include "endian.h"
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

// Calculates the flag byte for the event based on whether the event has an
// action and whether the event has data.
sky_event_flag_t sky_event_get_flag(sky_action_id_t action_id, uint32_t data_length)
{
    sky_event_flag_t flag = 0;

    if(action_id != 0) {
        flag |= SKY_EVENT_FLAG_ACTION;
    }
    if(data_length > 0) {
        flag |= SKY_EVENT_FLAG_DATA;
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
void clear_data(sky_event *event)
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
int allocate_data(sky_event *event)
{
    event->data_count++;
    event->data = realloc(event->data, sizeof(sky_event_data*) * event->data_count);
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
int deallocate_data(sky_event *event)
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
        event->data = realloc(event->data, sizeof(sky_event_data*) * event->data_count);
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
// timestamp - When the event occurred (in microseconds since midnight Jan 1,
//             1970 UTC).
// object_id - The identifier for the object that the event is related to.
// action    - The name of the action that was performed.
//
// Returns a reference to the new event.
sky_event *sky_event_create(sky_object_id_t object_id,
                            sky_timestamp_t timestamp,
                            sky_action_id_t action_id)
{
    sky_event *event;
    
    event = malloc(sizeof(sky_event)); check_mem(event);
    
    event->timestamp = timestamp;
    event->object_id = object_id;
    event->action_id = action_id;

    event->data = NULL;
    event->data_count = 0;

    return event;
    
error:
    sky_event_free(event);
    return NULL;
}

// Removes an event reference from memory.
//
// event - The event to free.
void sky_event_free(sky_event *event)
{
    if(event) {
        // Destroy data.
        uint32_t i=0;
        for(i=0; i<event->data_count; i++) {
            sky_event_data_free(event->data[i]);
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
int sky_event_copy(sky_event *source, sky_event **target)
{
    int rc;
    sky_event_data *data;

    check(source != NULL, "Source event is required for copy");

    // Copy basic properties.
    sky_event *event = sky_event_create(source->object_id, source->timestamp, source->action_id);
    
    // Copy event data.
    if(source->data_count > 0) {
        // Allocate memory for data.
        event->data_count = source->data_count;
        event->data = malloc(sizeof(sky_event_data*) * event->data_count);
        check_mem(event->data);

        // Copy each event data item.
        uint64_t i;
        for(i=0; i<event->data_count; i++) {
            rc = sky_event_data_copy(source->data[i], &data);
            check(rc == 0, "Unable to copy event data");
            event->data[i] = data;
        }
    }

    // Return event to the caller.
    *target = event;
    
    return 0;
    
error:
    if(event) sky_event_free(event);
    *target = NULL;

    return -1;
}



//======================================
// Sizeof
//======================================

// Calculates the total number of bytes needed to store an event.
//
// event - The event.
size_t sky_event_sizeof(sky_event *event)
{
    size_t sz = 0;
    
    // Add event flag.
    sz += sizeof(sky_event_flag_t);
    
    // Add timestamp.
    sz += sizeof(event->timestamp);
    
    // Add action if one is set.
    if(event->action_id != 0) {
        sz += sizeof(event->action_id);
    }
    
    // Add data if set.
    sky_event_data_length_t data_length = sky_event_sizeof_data(event);
    if(data_length > 0) {
        sz += sizeof(data_length);
        sz += data_length;
    }

    return sz;
}

// Calculates the total number of bytes needed to store just the data section
// of the event.
sky_event_data_length_t sky_event_sizeof_data(sky_event *event)
{
    size_t sz = 0;
    
    // Add size for each data item.
    uint64_t i;
    for(i=0; i<event->data_count; i++) {
        sz += sky_event_data_sizeof(event->data[i]);
    }
    
    return sz;
}

// Calculates the total length of an event element stored in raw format at the
// given pointer.
//
// ptr - A pointer to the raw event data.
//
// Returns the length of the raw event data.
size_t sky_event_sizeof_raw(void *ptr)
{
    size_t sz = 0;
    char event_flag = *((sky_event_flag_t*)ptr);

    // Add event flag and timestamp.
    sz += sizeof(sky_event_flag_t);
    sz += sizeof(sky_timestamp_t);

    // Add action length.
    if(event_flag & SKY_EVENT_FLAG_ACTION) {
        sz += sizeof(sky_action_id_t);
    }

    // Add data length.
    if(event_flag & SKY_EVENT_FLAG_DATA) {
        sky_event_data_length_t data_length = *((sky_event_data_length_t*)(ptr+sz));
        sz += sizeof(data_length);
        sz += data_length;
    }
    
    return sz;
}    



//======================================
// Pack
//======================================

// Serializes an event to memory at a given pointer location.
//
// event - The event to pack.
// ptr   - The pointer to the current location.
// sz    - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_pack(sky_event *event, void *ptr, size_t *sz)
{
    int rc;
    size_t _sz;
    void *start = ptr;
    
    // Validate.
    check(event != NULL, "Event required");
    check(ptr != NULL, "Pointer required");
    
    // Pack header.
    size_t data_length = sky_event_sizeof_data(event);
    rc = sky_event_pack_hdr(event->timestamp, event->action_id, data_length, ptr, &_sz);
    check(rc == 0, "Unable to pack event header");
    ptr += _sz;

    // Pack data.
    uint64_t i;
    for(i=0; i<event->data_count; i++) {
        rc = sky_event_data_pack(event->data[i], ptr, &_sz);
        check(rc == 0, "Unable to pack event data at %p", ptr);
        ptr += _sz;
    }
    
    // Store number of bytes written.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}

// Serializes the header of an event to memory at a given pointer location.
//
// timestamp   - The timestamp of the event.
// action_id   - The action id of the event.
// data_length - The length, in bytes, of the data section of the event.
// ptr         - The pointer to the current location.
// sz          - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_pack_hdr(sky_timestamp_t timestamp,
                       sky_action_id_t action_id,
                       sky_event_data_length_t data_length,
                       void *ptr, size_t *sz)
{
    void *start = ptr;
    
    // Validate.
    check(ptr != NULL, "Pointer required");
    
    // Write event flag.
    sky_event_flag_t flag = sky_event_get_flag(action_id, data_length);
    *((sky_event_flag_t*)ptr) = flag;
    ptr += sizeof(flag);
    
    // Write timestamp.
    *((sky_timestamp_t*)ptr) = timestamp;
    ptr += sizeof(sky_timestamp_t);
    
    // Write action id.
    if(action_id != 0) {
        *((sky_action_id_t*)ptr) = action_id;
        ptr += sizeof(sky_action_id_t);
    }

    // Write data length.
    if(data_length > 0) {
        *((sky_event_data_length_t*)ptr) = data_length;
        ptr += sizeof(data_length);
    }
    
    // Store number of bytes written.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}

// Deserializes an event from a given file at the file's current offset.
//
// event - The event to unpack into.
// ptr   - The pointer to the current location.
// sz    - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_unpack(sky_event *event, void *ptr, size_t *sz)
{
    int rc;
    size_t _sz;
    void *start = ptr;

    // Validate.
    check(event != NULL, "Event required");
    check(ptr != NULL, "Pointer required");

    // Read event header.
    sky_event_data_length_t data_length;
    rc = sky_event_unpack_hdr(&event->timestamp, &event->action_id, &data_length, ptr, &_sz);
    check(rc == 0, "Unable to unpack event header");
    ptr += _sz;

    // Clear existing data.
    clear_data(event);
    
    // Unpack data.
    int index = 0;
    void *endptr = ptr + data_length;
    while(ptr < endptr) {
        check(allocate_data(event) == 0, "Unable to append event data");
        event->data[index] = sky_event_data_create(0, NULL);

        rc = sky_event_data_unpack(event->data[index], ptr, &_sz);
        check(rc == 0, "Unable to unpack event data at %p", ptr);
        ptr += _sz;

        index++;
    }

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = (ptr-start);
    }

    return 0;

error:
    *sz = 0;
    return -1;
}

// Deserializes an event header from memory.
//
// timestamp   - A pointer to where the event timestamp will be returned.
// action_id   - A pointer to where the event's action id will be returned.
// data_length - A pointer to where the event's data length will be returned.
// ptr         - The pointer to the current location.
// sz          - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_unpack_hdr(sky_timestamp_t *timestamp,
                         sky_action_id_t *action_id,
                         sky_event_data_length_t *data_length,
                         void *ptr, size_t *sz)
{
    void *start = ptr;

    // Validate.
    check(ptr != NULL, "Pointer required");

    // Read event flag.
    sky_event_flag_t flag = *((sky_event_flag_t*)ptr);
    ptr += sizeof(flag);

    // Read timestamp.
    *timestamp = *((sky_timestamp_t*)ptr);
    ptr += sizeof(sky_timestamp_t);
    
    // Read action if one exists.
    if(flag & SKY_EVENT_FLAG_ACTION) {
        *action_id = *((sky_action_id_t*)ptr);
        ptr += sizeof(sky_action_id_t);
    }
    else {
        *action_id = 0;
    }

    // Read data length.
    if(flag & SKY_EVENT_FLAG_DATA) {
        *data_length = *((sky_event_data_length_t*)ptr);
        ptr += sizeof(sky_event_data_length_t);
    }
    else {
        *data_length = 0;
    }

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = (ptr-start);
    }

    return 0;

error:
    *sz = 0;
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
int sky_event_get_data(sky_event *event, sky_property_id_t key,
                       sky_event_data **data)
{
    uint64_t i;
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
int sky_event_set_data(sky_event *event, sky_property_id_t key, bstring value)
{
    int rc;
    sky_event_data *data = NULL;
    
    // Validation.
    check(event != NULL, "Event required");
    
    // Find data with existing key.
    rc = sky_event_get_data(event, key, &data);
    check(rc == 0, "Unable to find event data")
    
    // If existing key is found then update value.
    if(data != NULL) {
        // Cleanup old value.
        if(data->value != NULL) {
            bdestroy(data->value);
        }
    
        // Assign new value.
        data->value = bstrcpy(value);
        if(value != NULL) check_mem(data->value);
    }
    // Otherwise append a new data item.
    else {
        check(allocate_data(event) == 0, "Unable to allocate event data");
        event->data[event->data_count-1] = sky_event_data_create(key, value);
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
int sky_event_unset_data(sky_event *event, sky_property_id_t key)
{
    uint64_t i, j;
    
    // Validation.
    check(event != NULL, "Event required");

    // Loop over data to find matching key.
    for(i=0; i<event->data_count; i++) {
        // If found then unload item and resize array down.
        if(event->data[i]->key == key) {
            // Destroy data.
            sky_event_data_free(event->data[i]);
            
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
