#include <stdlib.h>
#include <arpa/inet.h>

#include "types.h"
#include "message.h"
#include "endian.h"
#include "mem.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Header
//======================================

// Creates a message header object in memory.
sky_message_header *sky_message_header_create()
{
    sky_message_header *header = NULL;
    header = calloc(1, sizeof(sky_message_header)); check_mem(header);

    return header;

error:
    sky_message_header_free(header);
    return NULL;
}

// Frees a message header from memory.
//
// header - The header object to be freed.
void sky_message_header_free(sky_message_header *header)
{
    if(header) {
        free(header);
    }
}

// Retrieves the message header from memory.
//
// ptr     - A pointer to the start of the message.
// header  - A reference to the variable that will store the header.
//
// Returns 0 if successful, otherwise returns -1.
int sky_message_header_parse(void *ptr, sky_message_header *header)
{
    // Parse version.
    memread(ptr, &header->version, sizeof(header->version), "message version");
    header->version = ntohs(header->version);

    // Parse type.
    memread(ptr, &header->type, sizeof(header->type), "message type");
    header->type = ntohl(header->type);

    // Parse length.
    memread(ptr, &header->length, sizeof(header->length), "message length");
    header->length = ntohl(header->length);

    return 0;

error:
    return -1;
}



//======================================
// EADD
//======================================

// Creates an EADD message object.
sky_eadd_message *sky_eadd_message_create()
{
    sky_eadd_message *message = NULL;
    message = calloc(1, sizeof(sky_eadd_message)); check_mem(message);
    
    return message;

error:
    sky_eadd_message_free(message);
    return NULL;
}

// Frees an EADD message object from memory.
//
// message - The message object to be freed.
void sky_eadd_message_free(sky_eadd_message *message)
{
    if(message) {
        uint16_t i;
        for(i=0; i<message->data_count; i++) {
            bdestroy(message->data_keys[i]);
            message->data_keys[i] = NULL;
            bdestroy(message->data_values[i]);
            message->data_values[i] = NULL;
        }
        if(message->data_keys) free(message->data_keys);
        if(message->data_values) free(message->data_values);
        free(message);
    }
}


// Parses an EADD message.
//
// ptr     - A pointer to the start of the message.
// length  - The length of the message.
// message - A reference to where the message will be stored.
//
// Returns 0 if successful, otherwise returns -1.
int sky_eadd_message_parse(void *ptr, sky_eadd_message *message)
{
    // Parse header.
    sky_message_header *header = sky_message_header_create();
    check(sky_message_header_parse(ptr, header) == 0, "Unable to parse header");
    ptr += SKY_MESSAGE_HEADER_LENGTH;

    // Determine end of message.
    void *maxptr = ptr + header->length;
    
    // TODO: Check for EOF of message before each read.

    // Parse database name.
    uint8_t database_name_length;
    memread(ptr, &database_name_length, sizeof(database_name_length), "message database name length");
    memread_bstr(ptr, message->database_name, database_name_length, "message database name");

    // Parse table name.
    uint8_t table_name_length;
    memread(ptr, &table_name_length, sizeof(table_name_length), "message table name length");
    memread_bstr(ptr, message->table_name, table_name_length, "message table name");

    // Parse object id.
    memread(ptr, &message->object_id, sizeof(message->object_id), "message object id");
    message->object_id = ntohll(message->object_id);
    
    // Parse timestamp.
    memread(ptr, &message->timestamp, sizeof(message->timestamp), "message timestamp");
    message->timestamp = ntohll(message->timestamp);

    // Parse action.
    uint16_t action_name_length;
    memread(ptr, &action_name_length, sizeof(action_name_length), "message action name length");
    action_name_length = ntohs(action_name_length);
    memread_bstr(ptr, message->action_name, action_name_length, "message action name");

    // Parse data count.
    memread(ptr, &message->data_count, sizeof(message->data_count), "message data count");
    message->data_count = ntohs(message->data_count);
    
    // Allocate space for data.
    message->data_keys = calloc(1, sizeof(bstring) * message->data_count);
    check_mem(message->data_keys);

    message->data_values = calloc(1, sizeof(bstring) * message->data_count);
    check_mem(message->data_values);
    
    // Parse data.
    uint32_t i;
    for(i=0; i<message->data_count; i++) {
        uint16_t data_key_length;
        memread(ptr, &data_key_length, sizeof(data_key_length), "message data key length");
        data_key_length = ntohs(data_key_length);
        memread_bstr(ptr, message->data_keys[i], data_key_length, "message data key");
        
        uint8_t data_value_length;
        memread(ptr, &data_value_length, sizeof(data_value_length), "message data value length");
        check(data_value_length < 128, "Data value cannot exceed 127 bytes");
        memread_bstr(ptr, message->data_values[i], data_value_length, "message data value");
    }
    
    free(header);
    
    return 0;

error:
    if(header) free(header);
    return -1;
}


