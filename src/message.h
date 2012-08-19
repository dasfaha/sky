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
    


//--------------------------------------
// Message Types
//--------------------------------------

#define SKY_MESSAGE_TYPE_EVENT 0x10000
#define SKY_MESSAGE_TYPE_PATH  0x20000

#define SKY_MESSAGE_TYPE_ADD   0x00001
#define SKY_MESSAGE_TYPE_UPD   0x00002
#define SKY_MESSAGE_TYPE_DEL   0x00003
#define SKY_MESSAGE_TYPE_GET   0x00004
#define SKY_MESSAGE_TYPE_ALL   0x00005
#define SKY_MESSAGE_TYPE_EACH  0x00006


//--------------------------------------
// Event Commands
//--------------------------------------

#define SKY_MESSAGE_EADD (SKY_MESSAGE_TYPE_EVENT + SKY_MESSAGE_TYPE_ADD)


//--------------------------------------
// Path Commands
//--------------------------------------

#define SKY_MESSAGE_PEACH (SKY_MESSAGE_TYPE_PATH + SKY_MESSAGE_TYPE_EACH)


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
    bstring database_name;
    bstring table_name;
    sky_object_id_t object_id;
    sky_timestamp_t timestamp;
    sky_action_id_t action_id;
} sky_eadd_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Header
//======================================

sky_message_header *sky_message_header_create();

void sky_message_header_free(sky_message_header *header);

int sky_message_header_parse(void *ptr, sky_message_header *header);


//======================================
// EADD
//======================================

sky_eadd_message *sky_eadd_message_create();

void sky_eadd_message_free(sky_eadd_message *message);

int sky_eadd_message_parse(void *ptr, sky_eadd_message *message);


#endif
