#ifndef _sky_aget_message_h
#define _sky_aget_message_h

#include <inttypes.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "bstring.h"
#include "table.h"
#include "event.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

// A message for retrieving an action by id from a table.
typedef struct sky_aget_message {
    sky_action_id_t action_id;
} sky_aget_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_aget_message *sky_aget_message_create();

void sky_aget_message_free(sky_aget_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_aget_message_pack(sky_aget_message *message, FILE *file);

int sky_aget_message_unpack(sky_aget_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_aget_message_process(sky_aget_message *message, sky_table *table,
    FILE *output);

#endif
