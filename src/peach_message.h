#ifndef _sky_peach_message_h
#define _sky_peach_message_h

#include <stdio.h>
#include <inttypes.h>

#include "bstring.h"
#include "types.h"
#include "table.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

// A message for querying each path in the database.
typedef struct {
    bstring query;
} sky_peach_message;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_peach_message *sky_peach_message_create();

void sky_peach_message_free(sky_peach_message *message);

//--------------------------------------
// Serialization
//--------------------------------------

int sky_peach_message_pack(sky_peach_message *message, FILE *file);

int sky_peach_message_unpack(sky_peach_message *message, FILE *file);

//--------------------------------------
// Processing
//--------------------------------------

int sky_peach_message_process(sky_peach_message *message, sky_table *table,
    FILE *output);

#endif
