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

// Frees an EADD message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_eadd_message_free(sky_eadd_message *message)
{
    if(message) {
        free(message);
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
    sz += minipack_sizeof_uint(message->object_id);
    sz += minipack_sizeof_int(message->timestamp);
    sz += minipack_sizeof_uint(message->action_id);
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
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Object ID
    minipack_fwrite_uint(file, message->object_id, &sz);
    check(sz != 0, "Unable to pack object id");

    // Timestamp
    minipack_fwrite_int(file, message->timestamp, &sz);
    check(sz != 0, "Unable to pack timestamp");

    // Action ID
    minipack_fwrite_uint(file, message->action_id, &sz);
    check(sz != 0, "Unable to unpack action id");
    
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
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Object ID
    message->object_id = (sky_object_id_t)minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack object id");

    // Timestamp
    message->timestamp = (sky_timestamp_t)minipack_fread_int(file, &sz);
    check(sz != 0, "Unable to unpack timestamp");

    // Action ID
    message->action_id = (sky_action_id_t)minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack action id");

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