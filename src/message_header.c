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
// Definition
//
//==============================================================================

#define SKY_MESSAGE_HEADER_ITEM_COUNT 6


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
    sky_message_header *header = calloc(1, sizeof(sky_message_header)); check_mem(header);
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
        if(header->name) bdestroy(header->name);
        header->name = NULL;

        if(header->database_name) bdestroy(header->database_name);
        header->database_name = NULL;

        if(header->table_name) bdestroy(header->table_name);
        header->table_name = NULL;

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
    sz += minipack_sizeof_array(SKY_MESSAGE_HEADER_ITEM_COUNT);
    sz += minipack_sizeof_uint(header->version);
    sz += minipack_sizeof_raw(blength(header->name));
    sz += blength(header->name);
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

    // Item count
    minipack_fwrite_array(file, SKY_MESSAGE_HEADER_ITEM_COUNT, &sz);
    check(sz != 0, "Unable to pack item count");

    // Version
    minipack_fwrite_uint(file, header->version, &sz);
    check(sz != 0, "Unable to pack version");

    // Message name
    rc = sky_minipack_fwrite_bstring(file, header->name);
    check(rc == 0, "Unable to pack name");

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

    // Item Count
    uint32_t count = minipack_fread_array(file, &sz);
    check(sz != 0, "Unable to unpack version");
    check(count == SKY_MESSAGE_HEADER_ITEM_COUNT, "Invalid header item count: %d; expected: %d", count, SKY_MESSAGE_HEADER_ITEM_COUNT);

    // Version
    header->version = minipack_fread_uint(file, &sz);
    check(sz != 0, "Unable to unpack version");

    // Message name
    rc = sky_minipack_fread_bstring(file, &header->name);
    check(rc == 0, "Unable to pack name");

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
