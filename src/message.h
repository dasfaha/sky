#ifndef _message_h
#define _message_h

#include <inttypes.h>

#include "bstring.h"
#include "types.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// This file handles the parsing of messages received by the server.


//==============================================================================
//
// Definitions
//
//==============================================================================

#define sky_message_version_t uint16_t

#define sky_message_type_t uint32_t

#define sky_message_length_t uint32_t


#define SKY_MESSAGE_HEADER_LENGTH \
    sizeof(sky_message_type_t) + \
    sizeof(sky_message_version_t) + \
    sizeof(sky_message_length_t)
    

#define SKY_MESSAGE_EADD 0x10001


//==============================================================================
//
// Typedefs
//
//==============================================================================

// The header info for a message.
typedef struct sky_message_header {
    sky_message_version_t version;
    sky_message_type_t type;
    sky_message_length_t length;
} sky_message_header;

// A message for adding events to the database.
typedef struct sky_eadd_message {
    sky_object_id_t object_id;
    sky_timestamp_t timestamp;
    bstring action_name;
    bstring *data_keys;
    bstring *data_values;
    uint16_t data_count;
} sky_eadd_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Header
//======================================

int sky_message_header_parse(void *ptr, sky_message_header *header);

void sky_message_header_free(sky_message_header *header);


//======================================
// EADD
//======================================

int sky_eadd_message_parse(void *ptr, sky_eadd_message *message);

void sky_eadd_message_free(sky_eadd_message *message);


#endif
