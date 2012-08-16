#include <stdlib.h>

#include "cursor.h"
#include "eql_path.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a path.
sky_eql_path *sky_eql_path_create()
{
    sky_eql_path *path = malloc(sizeof(sky_eql_path));
    path->path_ptr = NULL;
    return path;
}

// Frees a path.
//
// path - The path to free.
void sky_eql_path_free(sky_eql_path *path)
{
    if(path) {
        path->path_ptr = NULL;
        free(path);
    }
}


//======================================
// Cursor Management
//======================================

// Retrieves a cursor for the current path.
//
// path - The path.
//
// Returns a new cursor.
sky_eql_cursor *sky_eql_path_events(sky_eql_path *path)
{
    // Initialize cursor with path.
    sky_eql_cursor *cursor = sky_eql_cursor_create();
    sky_cursor_set_path(cursor->cursor, path->path_ptr);
    
    return cursor;
}
