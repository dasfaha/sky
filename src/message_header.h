#ifndef _sky_message_header_h
#define _sky_message_header_h

#include <stdio.h>
#include <inttypes.h>

#include "message_type.h"
#include "bstring.h"
#include "types.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

// The header info for a message.
typedef struct {
    uint64_t version;
    uint64_t type;
    uint64_t length;
    bstring database_name;
    bstring table_name;
} sky_message_header;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_message_header *sky_message_header_create();

void sky_message_header_free(sky_message_header *header);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_message_header_pack(sky_message_header *header, FILE *file);

int sky_message_header_unpack(sky_message_header *header, FILE *file);

#endif
