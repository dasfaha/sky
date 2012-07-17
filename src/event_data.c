#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "dbg.h"
#include "bstring.h"
#include "event.h"
#include "mem.h"
#include "minipack.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to event data.
//
// key   - The property id used as the key for the data.
// value - The string value of the data.
sky_event_data *sky_event_data_create(sky_property_id_t key, bstring value)
{
    sky_event_data *data;
    
    data = malloc(sizeof(sky_event_data));
    data->key = key;
    data->value = bstrcpy(value); if(value) check_mem(data->value);

    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Removes an event data reference from memory.
void sky_event_data_free(sky_event_data *data)
{
    if(data) {
        bdestroy(data->value);
        free(data);
    }
}

// Creates a copy of an event data item.
//
// source - The event data to copy.
// target - A reference to the new event data item.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_data_copy(sky_event_data *source, sky_event_data **target)
{
    check(source != NULL, "Event data source required for copy");

    *target = sky_event_data_create(source->key, source->value);

    return 0;
    
error:
    if(*target) sky_event_data_free(*target);
    *target = NULL;
    
    return -1;
}


//======================================
// Serialization
//======================================

// Calculates the total number of bytes needed to store an event data.
//
// data - The event data item.
size_t sky_event_data_sizeof(sky_event_data *data)
{
    size_t sz = 0;
    sz += sizeof(data->key);
    sz += minipack_sizeof_raw(blength(data->value));
    sz += blength(data->value);
    return sz;
}

// Serializes event data to memory at a given pointer.
//
// data   - The event data to pack.
// ptr    - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_data_pack(sky_event_data *data, void *ptr, size_t *sz)
{
    size_t _sz;
    void *start = ptr;

    // Validate.
    check(data != NULL, "Event data required");
    check(ptr != NULL, "Pointer required");
    
    // Write key.
    *((sky_property_id_t*)ptr) = data->key;
    ptr += sizeof(data->key);

    // Write value header.
    minipack_pack_raw(ptr, blength(data->value), &_sz);
    check(_sz != 0, "Unable to pack event data value header at %p", ptr);
    ptr += _sz;

    // Write value.
    memmove(ptr, bdata(data->value), blength(data->value));
    ptr += blength(data->value);

    // Store number of bytes written.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    return -1;
}

// Deserializes event data from memory at the current pointer.
//
// data - The event data to unpack into.
// ptr  - The pointer to the current location.
// sz   - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_data_unpack(sky_event_data *data, void *ptr, size_t *sz)
{
    size_t _sz;
    void *start = ptr;
    
    // Validate.
    check(data != NULL, "Event data required");
    check(ptr != NULL, "Pointer required");

    // Read key.
    data->key = *((sky_property_id_t*)ptr);
    ptr += sizeof(data->key);

    // Read value header.
    uint32_t value_length = minipack_unpack_raw(ptr, &_sz);
    check(_sz != 0, "Unable to unpack event value header at %p", ptr);
    ptr += _sz;

    // Read value.
    data->value = blk2bstr(ptr, value_length); check_mem(data->value);
    ptr += value_length;

    // Store number of bytes read.
    if(sz != NULL) {
        *sz = (ptr-start);
    }
    
    return 0;

error:
    *sz = 0;
    return -1;
}
