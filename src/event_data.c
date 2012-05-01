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

// Cleans the event data so that it conforms to max length standards.
void clean(sky_event_data *data)
{
    if(blength(data->value) > 127) {
        bstring tmp = bmidstr(data->value, 0, 127);
        bdestroy(data->value);
        data->value = tmp;
    }
}


//======================================
// Lifecycle
//======================================

// Creates a reference to event data.
//
// key   - The property id used as the key for the data.
// value - The string value of the data.
sky_event_data *sky_event_data_create(int16_t key, bstring value)
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
uint32_t sky_event_data_get_serialized_length(sky_event_data *data)
{
    uint32_t length = 0;

    clean(data);
    
    length += sizeof(data->key);
    length += sizeof(uint8_t);
    length += blength(data->value);

    return length;
}

// Serializes event data to memory at a given pointer.
//
// data   - The event data to serialize.
// addr   - The pointer to the current location.
// length - The number of bytes written.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_data_serialize(sky_event_data *data, void *addr, ptrdiff_t *length)
{
    void *start = addr;

    // Validate.
    check(data != NULL, "Event data required");
    check(addr != NULL, "Address required");
    
    // Clean data structure.
    clean(data);

    // Write key.
    memwrite(addr, &data->key, sizeof(data->key), "event data key");
    
    // Write value length.
    uint8_t value_length = blength(data->value);
    memwrite(addr, &value_length, sizeof(value_length), "event data value length");

    // Write value.
    memwrite_bstr(addr, data->value, value_length, "event data value");

    // Store number of bytes written.
    if(length != NULL) {
        *length = (addr-start);
    }
    
    return 0;

error:
    return -1;
}

// Deserializes event data from memory at the current pointer.
//
// data   - The event data to deserialize into.
// addr   - The pointer to the current location.
// length - The number of bytes read.
//
// Returns 0 if successful, otherwise returns -1.
int sky_event_data_deserialize(sky_event_data *data, void *addr, ptrdiff_t *length)
{
    void *start = addr;
    
    // Validate.
    check(data != NULL, "Event data required");
    check(addr != NULL, "Address required");

    // Read key.
    memread(addr, &data->key, sizeof(data->key), "event data key");
    
    // Read value length.
    uint8_t value_length;
    memread(addr, &value_length, sizeof(value_length), "event data value length");

    // Read value.
    memread_bstr(addr, data->value, value_length, "event data value");
    
    // Store number of bytes read.
    if(length != NULL) {
        *length = (addr-start);
    }
    
    return 0;

error:
    *length = 0;
    return -1;
}
