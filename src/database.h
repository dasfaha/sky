#ifndef _database_h
#define _database_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef struct sky_database {
    bstring path;
} sky_database;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_database *sky_database_create();

void sky_database_free(sky_database *database);


//--------------------------------------
// Path Management
//--------------------------------------

int sky_database_set_path(sky_database *database, bstring path);

#endif
