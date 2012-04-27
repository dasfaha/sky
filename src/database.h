#ifndef _database_h
#define _database_h

#include "bstring.h"

//==============================================================================
//
// Overview
//
//==============================================================================

/**
 * The database is a collection of object files. For more information on the
 * data storage format, read the object_file.c source.
 */


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Database {
    bstring path;
} Database;


//==============================================================================
//
// Functions
//
//==============================================================================

Database *Database_create(bstring path);

void Database_destroy(Database *database);


#endif
