#ifndef _database_h
#define _database_h

#include "bstring.h"

//==============================================================================
//
// Overview
//
//==============================================================================

/**
 * The database is a collection of tables. For more information on the
 * data storage format, read the table.c source.
 */


//==============================================================================
//
// Typedefs
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

sky_database *sky_database_create(bstring path);

void sky_database_free(sky_database *database);


#endif
