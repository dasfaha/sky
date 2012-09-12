#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "types.h"
#include "pall_message.h"
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

// Creates an PALL message object.
//
// Returns a new PALL message.
sky_pall_message *sky_pall_message_create()
{
    sky_pall_message *message = NULL;
    message = calloc(1, sizeof(sky_pall_message)); check_mem(message);
    return message;

error:
    sky_pall_message_free(message);
    return NULL;
}

// Frees an PALL message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_pall_message_free(sky_pall_message *message)
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
size_t sky_pall_message_sizeof(sky_pall_message *message)
{
    size_t sz = 0;
    check(message != NULL, "Message required");
    return sz;

error:
    return 0;
}

// Serializes an PALL message to a file stream.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pall_message_pack(sky_pall_message *message, FILE *file)
{
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    return 0;

error:
    return -1;
}

// Deserializes an PALL message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pall_message_unpack(sky_pall_message *message, FILE *file)
{
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Applies an PALL message to a table.
//
// message - The message.
// table   - The table to apply the message to.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_pall_message_process(sky_pall_message *message, sky_table *table,
                             FILE *output)
{
    size_t sz;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");
    check(output != NULL, "Output stream required");

    struct tagbstring status_str = bsStatic("status");
    struct tagbstring ok_str = bsStatic("ok");
    struct tagbstring properties_str = bsStatic("properties");

    // Return.
    //   {status:"OK", properties:[{...}]}
    minipack_fwrite_map(output, 2, &sz);
    check(sz > 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &status_str) == 0, "Unable to write status key");
    check(sky_minipack_fwrite_bstring(output, &ok_str) == 0, "Unable to write status value");
    check(sky_minipack_fwrite_bstring(output, &properties_str) == 0, "Unable to write properties key");

    // Loop over properties and serialize them.
    minipack_fwrite_array(output, table->property_file->property_count, &sz);
    check(sz > 0, "Unable to write properties array");
    
    uint32_t i;
    for(i=0; i<table->property_file->property_count; i++) {
        sky_property *property = table->property_file->properties[i];
        check(sky_property_pack(property, output) == 0, "Unable to write property");
    }
    
    return 0;

error:
    return -1;
}