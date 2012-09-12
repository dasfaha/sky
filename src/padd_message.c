#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "types.h"
#include "padd_message.h"
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

// Creates an PADD message object.
//
// Returns a new PADD message.
sky_padd_message *sky_padd_message_create()
{
    sky_padd_message *message = NULL;
    message = calloc(1, sizeof(sky_padd_message)); check_mem(message);
    message->property = sky_property_create(); check_mem(message->property);
    return message;

error:
    sky_padd_message_free(message);
    return NULL;
}

// Frees an PADD message object from memory.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_padd_message_free(sky_padd_message *message)
{
    if(message) {
        sky_padd_message_free_property(message);
        free(message);
    }
}

// Frees an property associated with an PADD message.
//
// message - The message object to be freed.
//
// Returns nothing.
void sky_padd_message_free_property(sky_padd_message *message)
{
    if(message) {
        // Only free the property if it's not managed by an property file.
        if(message->property && message->property->property_file == NULL) {
            sky_property_free(message->property);
        }
        message->property = NULL;
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
size_t sky_padd_message_sizeof(sky_padd_message *message)
{
    size_t sz = 0;
    sz += sky_property_sizeof(message->property);
    return sz;
}

// Serializes an PADD message to a file stream.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_padd_message_pack(sky_padd_message *message, FILE *file)
{
    int rc;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    rc = sky_property_pack(message->property, file);
    check(rc == 0, "Unable to pack property");
    
    return 0;

error:
    return -1;
}

// Deserializes an PADD message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_padd_message_unpack(sky_padd_message *message, FILE *file)
{
    int rc;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    rc = sky_property_unpack(message->property, file);
    check(rc == 0, "Unable to unpack property");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Applies an PADD message to a table.
//
// message - The message.
// table   - The table to apply the message to.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_padd_message_process(sky_padd_message *message, sky_table *table,
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

    // Add property.
    rc = sky_property_file_add_property(table->property_file, message->property);
    check(rc == 0, "Unable to add property");
    
    // Save property file.
    rc = sky_property_file_save(table->property_file);
    check(rc == 0, "Unable to save property file");
    
    // Return.
    //   {status:"OK", property:{...}}
    minipack_fwrite_map(output, 2, &sz);
    check(sz > 0, "Unable to write output");
    check(sky_minipack_fwrite_bstring(output, &status_str) == 0, "Unable to write status key");
    check(sky_minipack_fwrite_bstring(output, &ok_str) == 0, "Unable to write status value");
    check(sky_minipack_fwrite_bstring(output, &property_str) == 0, "Unable to write property key");
    check(sky_property_pack(message->property, output) == 0, "Unable to write property value");
    
    return 0;

error:
    return -1;
}