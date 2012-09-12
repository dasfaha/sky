#ifndef _sky_eadd_message_h
#define _sky_eadd_message_h

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

// A message for adding events to the database.
typedef struct sky_eadd_message {
    sky_object_id_t object_id;
    sky_timestamp_t timestamp;
    sky_action_id_t action_id;
} sky_eadd_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_eadd_message *sky_eadd_message_create();

void sky_eadd_message_free(sky_eadd_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_eadd_message_pack(sky_eadd_message *message, FILE *file);

int sky_eadd_message_unpack(sky_eadd_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_eadd_message_process(sky_eadd_message *message, sky_table *table,
    FILE *output);

#endif
