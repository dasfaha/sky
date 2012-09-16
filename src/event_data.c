#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "dbg.h"
#include "bstring.h"
#include "event.h"
#include "property.h"
#include "mem.h"
#include "minipack.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a reference to event data with no data type.
//
// key   - The property id used as the key for the data.
sky_event_data *sky_event_data_create(sky_property_id_t key)
{
    sky_event_data *data = calloc(1, sizeof(sky_event_data)); check_mem(data);
    data->key = key;
    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Creates a reference to event data representing an integer.
//
// key   - The property id used as the key for the data.
// value - The integer value of the data.
sky_event_data *sky_event_data_create_int(sky_property_id_t key, int64_t value)
{
    sky_event_data *data = calloc(1, sizeof(sky_event_data)); check_mem(data);
    data->key = key;
    data->data_type = &SKY_DATA_TYPE_INT;
    data->int_value = value;

    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Creates a reference to event data representing a float.
//
// key   - The property id used as the key for the data.
// value - The float value of the data.
sky_event_data *sky_event_data_create_float(sky_property_id_t key, double value)
{
    sky_event_data *data = calloc(1, sizeof(sky_event_data)); check_mem(data);
    data->key = key;
    data->data_type = &SKY_DATA_TYPE_FLOAT;
    data->float_value = value;

    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Creates a reference to event data representing a boolean.
//
// key   - The property id used as the key for the data.
// value - The boolean value of the data.
sky_event_data *sky_event_data_create_boolean(sky_property_id_t key, bool value)
{
    sky_event_data *data = calloc(1, sizeof(sky_event_data)); check_mem(data);
    data->key = key;
    data->data_type = &SKY_DATA_TYPE_BOOLEAN;
    data->boolean_value = value;

    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Creates a reference to event data.
//
// key   - The property id used as the key for the data.
// value - The string value of the data.
sky_event_data *sky_event_data_create_string(sky_property_id_t key, bstring value)
{
    sky_event_data *data = calloc(1, sizeof(sky_event_data)); check_mem(data);
    data->key = key;
    data->data_type = &SKY_DATA_TYPE_STRING;
    data->string_value = bstrcpy(value);
    if(value) check_mem(data->string_value);

    return data;
    
error:
    sky_event_data_free(data);
    return NULL;
}

// Removes an event data reference from memory.
//
// Returns nothing.
void sky_event_data_free(sky_event_data *data)
{
    if(data) {
        if(data->data_type == &SKY_DATA_TYPE_STRING) bdestroy(data->string_value);
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

    if(source->data_type == &SKY_DATA_TYPE_INT) {
        *target = sky_event_data_create_int(source->key, source->int_value);
    }
    else if(source->data_type == &SKY_DATA_TYPE_FLOAT) {
        *target = sky_event_data_create_int(source->key, source->float_value);
    }
    else if(source->data_type == &SKY_DATA_TYPE_BOOLEAN) {
        *target = sky_event_data_create_int(source->key, source->boolean_value);
    }
    else if(source->data_type == &SKY_DATA_TYPE_STRING) {
        *target = sky_event_data_create_string(source->key, source->string_value);
    }
    else {
        sentinel("Invalid data type for event data: '%s'", bdata(source->data_type));
    }

    return 0;
    
error:
    if(*target) sky_event_data_free(*target);
    *target = NULL;
    
    return -1;
}


//--------------------------------------
// Serialization
//--------------------------------------

// Calculates the total number of bytes needed to store an event data.
//
// data - The event data item.
size_t sky_event_data_sizeof(sky_event_data *data)
{
    size_t sz = 0;
    sz += sizeof(data->key);
    if(data->data_type == &SKY_DATA_TYPE_INT) {
        sz += minipack_sizeof_int(data->int_value);
    }
    else if(data->data_type == &SKY_DATA_TYPE_FLOAT) {
        sz += minipack_sizeof_double(data->float_value);
    }
    else if(data->data_type == &SKY_DATA_TYPE_BOOLEAN) {
        sz += minipack_sizeof_bool(data->boolean_value);
    }
    else if(data->data_type == &SKY_DATA_TYPE_STRING) {
        sz += minipack_sizeof_raw(blength(data->string_value));
        sz += blength(data->string_value);
    }
    else {
        sentinel("Invalid data type for event data: '%s'", bdata(data->data_type));
    }
    return sz;

error:
    return 0;
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

    // Write value.
    if(data->data_type == &SKY_DATA_TYPE_INT) {
        minipack_pack_int(ptr, data->int_value, &_sz);
        check(_sz != 0, "Unable to pack event data int value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_FLOAT) {
        minipack_pack_double(ptr, data->float_value, &_sz);
        check(_sz != 0, "Unable to pack event data float value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_BOOLEAN) {
        minipack_pack_bool(ptr, data->boolean_value, &_sz);
        check(_sz != 0, "Unable to pack event data boolean value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_STRING) {
        // Write value header.
        minipack_pack_raw(ptr, blength(data->string_value), &_sz);
        check(_sz != 0, "Unable to pack event data value header at %p", ptr);
        ptr += _sz;

        // Write actual string value.
        memmove(ptr, bdata(data->string_value), blength(data->string_value));
        ptr += blength(data->string_value);
    }
    else {
        sentinel("Invalid data type for event data: '%s'", bdata(data->data_type));
    }

    // Store number of bytes written.
    if(sz != NULL) *sz = (ptr-start);
    
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

    // If there is no data type set then determine it from the data.
    if(data->data_type == NULL) {
        if(minipack_is_raw(ptr)) {
            data->data_type = &SKY_DATA_TYPE_STRING;
        }
        else if(minipack_is_bool(ptr)) {
            data->data_type = &SKY_DATA_TYPE_BOOLEAN;
        }
        else if(minipack_is_double(ptr)) {
            data->data_type = &SKY_DATA_TYPE_FLOAT;
        }
        else {
            data->data_type = &SKY_DATA_TYPE_INT;
        }
    }

    // Read value.
    if(data->data_type == &SKY_DATA_TYPE_INT) {
        data->int_value = minipack_unpack_int(ptr, &_sz);
        check(_sz != 0, "Unable to unpack event int value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_FLOAT) {
        data->float_value = minipack_unpack_double(ptr, &_sz);
        check(_sz != 0, "Unable to unpack event float value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_BOOLEAN) {
        data->boolean_value = minipack_unpack_bool(ptr, &_sz);
        check(_sz != 0, "Unable to unpack event boolean value");
        ptr += _sz;
    }
    else if(data->data_type == &SKY_DATA_TYPE_STRING) {
        // Read raw header.
        uint32_t value_length = minipack_unpack_raw(ptr, &_sz);
        check(_sz != 0, "Unable to unpack event value header at %p", ptr);
        ptr += _sz;

        // Read raw data.
        data->string_value = blk2bstr(ptr, value_length); check_mem(data->string_value);
        ptr += value_length;
    }
    else {
        sentinel("Invalid data type for event data: '%s'", bdata(data->data_type));
    }

    // Store number of bytes read.
    if(sz != NULL) *sz = (ptr-start);
    
    return 0;

error:
    *sz = 0;
    return -1;
}
