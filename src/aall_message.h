#ifndef _sky_aall_message_h
#define _sky_aall_message_h

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
typedef struct sky_aall_message {
    int64_t dummy;
} sky_aall_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_aall_message *sky_aall_message_create();

void sky_aall_message_free(sky_aall_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_aall_message_pack(sky_aall_message *message, FILE *file);

int sky_aall_message_unpack(sky_aall_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_aall_message_process(sky_aall_message *message, sky_table *table,
    FILE *output);

#endif
