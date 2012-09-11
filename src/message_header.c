#include <stdlib.h>
#include <arpa/inet.h>

#include "types.h"
#include "minipack.h"
#include "message_header.h"
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

// Creates a message header object in memory.
//
// Returns a message header object.
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


//--------------------------------------
// Serialization
//--------------------------------------

// Calculates the total number of bytes needed to store the message header.
//
// message - The message.
//
// Returns the number of bytes required to store the message.
size_t sky_message_header_sizeof(sky_message_header *header)
{
    size_t sz = 0;
    sz += minipack_sizeof_uint(header->version);
    sz += minipack_sizeof_uint(header->type);
    sz += minipack_sizeof_uint(header->length);
    sz += minipack_sizeof_raw(blength(header->database_name));
    sz += blength(header->database_name);
    sz += minipack_sizeof_raw(blength(header->table_name));
    sz += blength(header->database_name);
    return sz;
}

// Serializes a message header to a file stream.
//
// header - The header.
// file   - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_message_header_pack(sky_message_header *header, FILE *file)
{
    int rc;
    size_t sz;
    check(header != NULL, "Header required");
    check(file != NULL, "File stream required");

    // Version
    minipack_fwrite_uint(file, header->version, &sz);
    check(sz != 0, "Unable to pack version");

    // Type
    minipack_fwrite_uint(file, header->type, &sz);
    check(sz != 0, "Unable to pack type");

    // Length
    minipack_fwrite_uint(file, header->length, &sz);
    check(sz != 0, "Unable to pack length");

    // Database name
    rc = sky_minipack_fwrite_bstring(file, header->database_name);
    check(rc == 0, "Unable to pack database name");

    // Table name
    rc = sky_minipack_fwrite_bstring(file, header->table_name);
    check(rc == 0, "Unable to pack table name");

    return 0;

error:
    return -1;
}

// Deserializes a message header from a file stream.
//
// header - The message header.
// file   - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_message_header_unpack(sky_message_header *header, FILE *file)
{
    int rc;
    size_t sz;
    check(header != NULL, "Header required");
    check(file != NULL, "Pointer required");

    // Version
    header->version = minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack version");

    // Type
    header->type = minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack type");

    // Length
    header->length = minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack message body length");

    // Database name
    rc = sky_minipack_fread_bstring(file, &header->database_name);
    check(rc == 0, "Unable to pack database name");

    // Table name
    rc = sky_minipack_fread_bstring(file, &header->table_name);
    check(rc == 0, "Unable to pack table name");

    return 0;

error:
    return -1;
}
