#ifndef _sky_padd_message_h
#define _sky_padd_message_h

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

// A message for adding properties to a table.
typedef struct sky_padd_message {
    sky_property* property;
} sky_padd_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_padd_message *sky_padd_message_create();

void sky_padd_message_free(sky_padd_message *message);

void sky_padd_message_free_property(sky_padd_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_padd_message_pack(sky_padd_message *message, FILE *file);

int sky_padd_message_unpack(sky_padd_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_padd_message_process(sky_padd_message *message, sky_table *table,
    FILE *output);

#endif
