#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "types.h"
#include "eadd_message.h"
#include "minipack.h"
#include "endian.h"
#include "mem.h"
#include "dbg.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

#define SKY_EADD_KEY_COUNT 4

struct tagbstring SKY_EADD_KEY_OBJECT_ID = bsStatic("objectId");

struct tagbstring SKY_EADD_KEY_TIMESTAMP = bsStatic("timestamp");

struct tagbstring SKY_EADD_KEY_ACTION_ID = bsStatic("actionId");

struct tagbstring SKY_EADD_KEY_DATA = bsStatic("data");


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

size_t sky_eadd_message_sizeof_data(sky_eadd_message *message);

int sky_eadd_message_pack_data(sky_eadd_message *message, FILE *file);

int sky_eadd_message_unpack_data(sky_eadd_message *message, FILE *file);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an EADD message object.
//
// Returns a new EADD message.
sky_eadd_message *sky_eadd_message_create()
{
    sky_eadd_message *message = NULL;
    message = calloc(1, sizeof(sky_eadd_message)); check_mem(message);
    return message;

error:
    sky_eadd_message_free(message);
    return NULL;
}

// Creates an EADD message data object.
//
// Returns a new EADD message data.
sky_eadd_message_data *sky_eadd_message_data_create()
{
    sky_eadd_message_data *data = NULL;
    data = calloc(1, sizeof(sky_eadd_message_data)); check_mem(data);
    return data;

error:
    sky_eadd_message_data_free(data);
    return NULL;
}

// Frees an EADD message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_eadd_message_free(sky_eadd_message *message)
{
    if(message) {
        uint32_t i;
        for(i=0; i<message->data_count; i++) {
            sky_eadd_message_data_free(message->data[i]);
            message->data[i] = NULL;
        }
        free(message->data);
        message->data = NULL;
        
        free(message);
    }
}

// Frees an EADD message data object from memory.
//
// data - The message data object to be freed.
//
// Returns nothing.
void sky_eadd_message_data_free(sky_eadd_message_data *data)
{
    if(data) {
        bdestroy(data->key);
        data->key = NULL;
        if(data->data_type == &SKY_DATA_TYPE_STRING) {
            bdestroy(data->string_value);
            data->string_value = NULL;
        }
        data->data_type = NULL;
        free(data);
    }
}


//--------------------------------------
// Serialization
//--------------------------------------

// Calculates the total number of bytes needed to store the message.
//
// message - The message.
//
// Returns the number of bytes required to store the message.
size_t sky_eadd_message_sizeof(sky_eadd_message *message)
{
    size_t sz = 0;
    sz += minipack_sizeof_map(SKY_EADD_KEY_COUNT);
    sz += minipack_sizeof_raw(blength(&SKY_EADD_KEY_OBJECT_ID)) + blength(&SKY_EADD_KEY_OBJECT_ID);
    sz += minipack_sizeof_uint(message->object_id);
    sz += minipack_sizeof_raw(blength(&SKY_EADD_KEY_TIMESTAMP)) + blength(&SKY_EADD_KEY_TIMESTAMP);
    sz += minipack_sizeof_int(message->timestamp);
    sz += minipack_sizeof_raw(blength(&SKY_EADD_KEY_ACTION_ID)) + blength(&SKY_EADD_KEY_ACTION_ID);
    sz += minipack_sizeof_uint(message->action_id);
    sz += minipack_sizeof_raw(blength(&SKY_EADD_KEY_DATA)) + blength(&SKY_EADD_KEY_DATA);
    sz += sky_eadd_message_sizeof_data(message);
    return sz;
}

// Calculates the total number of bytes needed to store the data property of
// the message.
//
// message - The message.
//
// Returns the number of bytes required to store the data property of the
// message.
size_t sky_eadd_message_sizeof_data(sky_eadd_message *message)
{
    uint32_t i;
    size_t sz = 0;
    sz += minipack_sizeof_map(message->data_count);
    for(i=0; i<message->data_count; i++) {
        sky_eadd_message_data *data = message->data[i];
        sz += minipack_sizeof_raw(blength(data->key)) + blength(data->key);
        
        if(data->data_type == &SKY_DATA_TYPE_STRING) {
            sz += minipack_sizeof_raw(blength(data->string_value)) + blength(data->string_value);
        }
        else if(data->data_type == &SKY_DATA_TYPE_INT) {
            sz += minipack_sizeof_int(data->int_value);
        }
        else if(data->data_type == &SKY_DATA_TYPE_FLOAT) {
            sz += minipack_sizeof_double(data->float_value);
        }
        else if(data->data_type == &SKY_DATA_TYPE_BOOLEAN) {
            sz += minipack_sizeof_bool(data->boolean_value);
        }
    }
    return sz;
}

// Serializes an EADD message to a file stream.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_pack(sky_eadd_message *message, FILE *file)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Map
    minipack_fwrite_map(file, SKY_EADD_KEY_COUNT, &sz);
    check(sz > 0, "Unable to write map");
    
    // Object ID
    check(sky_minipack_fwrite_bstring(file, &SKY_EADD_KEY_OBJECT_ID) == 0, "Unable to pack object id key");
    minipack_fwrite_int(file, message->object_id, &sz);
    check(sz != 0, "Unable to pack object id");

    // Timestamp
    check(sky_minipack_fwrite_bstring(file, &SKY_EADD_KEY_TIMESTAMP) == 0, "Unable to pack timestamp key");
    minipack_fwrite_int(file, message->timestamp, &sz);
    check(sz != 0, "Unable to pack timestamp");

    // Action ID
    check(sky_minipack_fwrite_bstring(file, &SKY_EADD_KEY_ACTION_ID) == 0, "Unable to pack action_id key");
    minipack_fwrite_int(file, message->action_id, &sz);
    check(sz != 0, "Unable to pack action id");
    
    // Data
    check(sky_minipack_fwrite_bstring(file, &SKY_EADD_KEY_DATA) == 0, "Unable to pack data key");
    rc = sky_eadd_message_pack_data(message, file);
    check(rc == 0, "Unable to pack eadd data");
    
    return 0;

error:
    return -1;
}

// Serializes the data map of an EADD message.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_pack_data(sky_eadd_message *message, FILE *file)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Map
    minipack_fwrite_map(file, message->data_count, &sz);
    check(sz > 0, "Unable to write map");
    
    // Map items
    uint32_t i;
    for(i=0; i<message->data_count; i++) {
        sky_eadd_message_data *data = message->data[i];
        
        // Write key.
        rc = sky_minipack_fwrite_bstring(file, data->key);
        check(rc == 0, "Unable to pack data key");
        
        // Write in the appropriate data type.
        if(data->data_type == &SKY_DATA_TYPE_STRING) {
            rc = sky_minipack_fwrite_bstring(file, data->string_value);
            check(rc == 0, "Unable to pack string value");
        }
        else if(data->data_type == &SKY_DATA_TYPE_INT) {
            minipack_fwrite_int(file, data->int_value, &sz);
            check(sz > 0, "Unable to pack int value");
        }
        else if(data->data_type == &SKY_DATA_TYPE_FLOAT) {
            minipack_fwrite_double(file, data->float_value, &sz);
            check(sz > 0, "Unable to pack float value");
        }
        else if(data->data_type == &SKY_DATA_TYPE_BOOLEAN) {
            minipack_fwrite_bool(file, data->boolean_value, &sz);
            check(sz > 0, "Unable to pack boolean value");
        }
        else {
            sentinel("Unsupported data type in eadd data message struct");
        }
    }

    return 0;

error:
    return -1;
}

// Deserializes an EADD message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_unpack(sky_eadd_message *message, FILE *file)
{
    int rc;
    size_t sz;
    bstring key = NULL;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Map
    uint32_t map_length = minipack_fread_map(file, &sz);
    check(sz > 0, "Unable to read map");
    
    // Map items
    uint32_t i;
    for(i=0; i<map_length; i++) {
        rc = sky_minipack_fread_bstring(file, &key);
        check(rc == 0, "Unable to read map key");
        
        if(biseq(key, &SKY_EADD_KEY_OBJECT_ID) == 1) {
            message->object_id = (sky_object_id_t)minipack_fread_uint(file, &sz);
            check(sz != 0, "Unable to unpack object id");
        }
        else if(biseq(key, &SKY_EADD_KEY_TIMESTAMP) == 1) {
            message->timestamp = (sky_timestamp_t)minipack_fread_int(file, &sz);
            check(sz != 0, "Unable to unpack timestamp");
        }
        else if(biseq(key, &SKY_EADD_KEY_ACTION_ID) == 1) {
            message->action_id = (sky_action_id_t)minipack_fread_uint(file, &sz);
            check(sz != 0, "Unable to unpack action id");
        }
        else if(biseq(key, &SKY_EADD_KEY_DATA) == 1) {
            rc = sky_eadd_message_unpack_data(message, file);
            check(rc == 0, "Unable to unpack eadd data value");
        }
        
        bdestroy(key);
    }

    return 0;

error:
    bdestroy(key);
    return -1;
}

// Deserializes the data map of an EADD message.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_unpack_data(sky_eadd_message *message, FILE *file)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Map
    uint32_t map_length = minipack_fread_map(file, &sz);
    check(sz > 0, "Unable to read map");
    
    // Allocate data array.
    message->data = calloc(1, sizeof(*message->data) * map_length); check_mem(message->data);
    
    // Map items
    uint32_t i;
    for(i=0; i<map_length; i++) {
        sky_eadd_message_data *data = sky_eadd_message_data_create(); check_mem(data);
        
        rc = sky_minipack_fread_bstring(file, &data->key);
        check(rc == 0, "Unable to read data key");
        
        // Read the first byte of the message to determine the type.
        uint8_t buffer[1];
        check(fread(buffer, sizeof(*buffer), 1, file) == 1, "Unable to read data type");
        ungetc(buffer[0], file);
        
        // Read in the appropriate data type.
        if(minipack_is_raw((void*)buffer)) {
            data->data_type = &SKY_DATA_TYPE_STRING;
            rc = sky_minipack_fread_bstring(file, &data->string_value);
            check(rc == 0, "Unable to unpack string value");
        }
        else if(minipack_is_bool((void*)buffer)) {
            data->data_type = &SKY_DATA_TYPE_BOOLEAN;
            data->boolean_value = minipack_fread_bool(file, &sz);
            check(sz != 0, "Unable to unpack boolean value");
        }
        else if(minipack_is_double((void*)buffer)) {
            data->data_type = &SKY_DATA_TYPE_FLOAT;
            data->float_value = minipack_fread_double(file, &sz);
            check(sz != 0, "Unable to unpack float value");
        }
        else {
            data->data_type = &SKY_DATA_TYPE_INT;
            data->int_value = minipack_fread_int(file, &sz);
            check(sz != 0, "Unable to unpack int value");
        }
    }

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Applies an EADD message to a table.
//
// message - The message.
// table   - The table to apply the message to.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_process(sky_eadd_message *message, sky_table *table,
                             FILE *output)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");
    check(output != NULL, "Output stream required");

    struct tagbstring status_str = bsStatic("status");
    struct tagbstring ok_str = bsStatic("ok");

    // Create event object.
    sky_event *event = sky_event_create(message->object_id, message->timestamp, message->action_id);
    
    // Add event to table.
    rc = sky_table_add_event(table, event);
    check(rc == 0, "Unable to add event to table");
    
    // Return {status:"OK"}
    check(minipack_fwrite_map(output, 1, &sz) == 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &status_str) == 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &ok_str) == 0, "Unable to write output");
    
    return 0;

error:
    return -1;
}