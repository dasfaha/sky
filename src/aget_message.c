#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "types.h"
#include "aget_message.h"
#include "action.h"
#include "minipack.h"
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

// Creates an AGET message object.
//
// Returns a new AGET message.
sky_aget_message *sky_aget_message_create()
{
    sky_aget_message *message = NULL;
    message = calloc(1, sizeof(sky_aget_message)); check_mem(message);
    return message;

error:
    sky_aget_message_free(message);
    return NULL;
}

// Frees an AGET message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_aget_message_free(sky_aget_message *message)
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
size_t sky_aget_message_sizeof(sky_aget_message *message)
{
    size_t sz = 0;
    sz += minipack_sizeof_uint(message->action_id);
    return sz;
}

// Serializes an AGET message to a file stream.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_aget_message_pack(sky_aget_message *message, FILE *file)
{
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    minipack_fwrite_uint(file, message->action_id, &sz);
    check(sz > 0, "Unable to pack action id");
    
    return 0;

error:
    return -1;
}

// Deserializes an AGET message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_aget_message_unpack(sky_aget_message *message, FILE *file)
{
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    message->action_id = minipack_fread_uint(file, &sz);
    check(sz > 0, "Unable to unpack action id");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Applies an AGET message to a table.
//
// message - The message.
// table   - The table to apply the message to.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_aget_message_process(sky_aget_message *message, sky_table *table,
                             FILE *output)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");
    check(output != NULL, "Output stream required");

    struct tagbstring status_str = bsStatic("status");
    struct tagbstring ok_str = bsStatic("ok");
    struct tagbstring action_str = bsStatic("action");

    // Retrieve action.
    sky_action *action = NULL;
    rc = sky_action_file_find_action_by_id(table->action_file, message->action_id, &action);
    check(rc == 0, "Unable to add action");
    
    // Return.
    //   {status:"OK", action:{...}}
    minipack_fwrite_map(output, 2, &sz);
    check(sz > 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &status_str) == 0, "Unable to write status key");
    check(sky_minipack_fwrite_bstring(output, &ok_str) == 0, "Unable to write status value");
    check(sky_minipack_fwrite_bstring(output, &action_str) == 0, "Unable to write action key");
    
    if(action != NULL) {
        check(sky_action_pack(action, output) == 0, "Unable to write action value");
    }
    else {
        minipack_fwrite_nil(output, &sz);
        check(sz > 0, "Unable to write null action value");
    }
    
    return 0;

error:
    return -1;
}