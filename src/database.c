#include <stdlib.h>

#include "dbg.h"
#include "bstring.h"
#include "database.h"

//==============================================================================
//
// Database Management
//
//==============================================================================

/*
 * Creates a reference to a database.
 *
 * path - The file path where the data resides on disk.
 */
Database *Database_create(bstring path)
{
    Database *database;

    check(path != NULL, "Cannot create database without a path");
    
    database = malloc(sizeof(Database));
    database->path = bstrcpy(path); check_mem(database->path);

    return database;
    
error:
    Database_destroy(database);
    return NULL;
}

/*
 * Removes a database reference from memory.
 */
void Database_destroy(Database *database)
{
    if(database) {
        bdestroy(database->path); database->path = NULL;
        free(database);
    }
}
