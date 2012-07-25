#include <stdlib.h>

#include "dbg.h"
#include "bstring.h"
#include "database.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a reference to a database.
//
// path - The file path where the data resides on disk.
//
// Returns a reference to the new database.
sky_database *sky_database_create()
{
    sky_database *database = calloc(sizeof(sky_database), 1);
    check_mem(database);
    return database;
    
error:
    sky_database_free(database);
    return NULL;
}

// Removes a database reference from memory.
//
// database - The database to free.
void sky_database_free(sky_database *database)
{
    if(database) {
        bdestroy(database->path); database->path = NULL;
        free(database);
    }
}


//--------------------------------------
// Path Management
//--------------------------------------

// Sets the file path of a database.
//
// database - The database.
// path     - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_database_set_path(sky_database *database, bstring path)
{
    check(database != NULL, "Database required");

    if(database->path) {
        bdestroy(database->path);
    }
    
    database->path = bstrcpy(path);
    if(path) check_mem(database->path);

    return 0;

error:
    database->path = NULL;
    return -1;
}
