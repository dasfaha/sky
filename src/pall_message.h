#ifndef _sky_pall_message_h
#define _sky_pall_message_h

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
typedef struct sky_pall_message {
    int64_t dummy;
} sky_pall_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_pall_message *sky_pall_message_create();

void sky_pall_message_free(sky_pall_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_pall_message_pack(sky_pall_message *message, FILE *file);

int sky_pall_message_unpack(sky_pall_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_pall_message_process(sky_pall_message *message, sky_table *table,
    FILE *output);

#endif
