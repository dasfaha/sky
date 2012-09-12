#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "types.h"
#include "pget_message.h"
#include "property.h"
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

// Creates an PGET message object.
//
// Returns a new PGET message.
sky_pget_message *sky_pget_message_create()
{
    sky_pget_message *message = NULL;
    message = calloc(1, sizeof(sky_pget_message)); check_mem(message);
    return message;

error:
    sky_pget_message_free(message);
    return NULL;
}

// Frees an PGET message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_pget_message_free(sky_pget_message *message)
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
size_t sky_pget_message_sizeof(sky_pget_message *message)
{
    size_t sz = 0;
    sz += minipack_sizeof_int(message->property_id);
    return sz;
}

// Serializes an PGET message to a file stream.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pget_message_pack(sky_pget_message *message, FILE *file)
{
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    minipack_fwrite_int(file, message->property_id, &sz);
    check(sz > 0, "Unable to pack property id");
    
    return 0;

error:
    return -1;
}

// Deserializes an PGET message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pget_message_unpack(sky_pget_message *message, FILE *file)
{
    size_t sz;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    message->property_id = (sky_property_id_t)minipack_fread_int(file, &sz);
    check(sz > 0, "Unable to unpack property id");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Applies an PGET message to a table.
//
// message - The message.
// table   - The table to apply the message to.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pget_message_process(sky_pget_message *message, sky_table *table,
                             FILE *output)
{
    int rc;
    size_t sz;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");
    check(output != NULL, "Output stream required");

    struct tagbstring status_str = bsStatic("status");
    struct tagbstring ok_str = bsStatic("ok");
    struct tagbstring property_str = bsStatic("property");

    // Retrieve property.
    sky_property *property = NULL;
    rc = sky_property_file_find_by_id(table->property_file, message->property_id, &property);
    check(rc == 0, "Unable to add property");
    
    // Return.
    //   {status:"OK", property:{...}}
    minipack_fwrite_map(output, 2, &sz);
    check(sz > 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &status_str) == 0, "Unable to write status key");
    check(sky_minipack_fwrite_bstring(output, &ok_str) == 0, "Unable to write status value");
    check(sky_minipack_fwrite_bstring(output, &property_str) == 0, "Unable to write property key");
    
    if(property != NULL) {
        check(sky_property_pack(property, output) == 0, "Unable to write property value");
    }
    else {
        minipack_fwrite_nil(output, &sz);
        check(sz > 0, "Unable to write null property value");
    }
    
    return 0;

error:
    return -1;
}