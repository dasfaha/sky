#ifndef _sky_pget_message_h
#define _sky_pget_message_h

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

// A message for retrieving a property by id from a table.
typedef struct sky_pget_message {
    sky_property_id_t property_id;
} sky_pget_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_pget_message *sky_pget_message_create();

void sky_pget_message_free(sky_pget_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_pget_message_pack(sky_pget_message *message, FILE *file);

int sky_pget_message_unpack(sky_pget_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_pget_message_process(sky_pget_message *message, sky_table *table,
    FILE *output);

#endif
