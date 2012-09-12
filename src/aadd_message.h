#ifndef _sky_aadd_message_h
#define _sky_aadd_message_h

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

// A message for adding actions to a table.
typedef struct sky_aadd_message {
    sky_action* action;
} sky_aadd_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_aadd_message *sky_aadd_message_create();

void sky_aadd_message_free(sky_aadd_message *message);

void sky_aadd_message_free_action(sky_aadd_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_aadd_message_pack(sky_aadd_message *message, FILE *file);

int sky_aadd_message_unpack(sky_aadd_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_aadd_message_process(sky_aadd_message *message, sky_table *table,
    FILE *output);

#endif
