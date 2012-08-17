#include <stdlib.h>

#include "eql_cursor.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a cursor.
sky_eql_cursor *sky_eql_cursor_create()
{
    sky_eql_cursor *cursor = malloc(sizeof(sky_eql_cursor));
    cursor->cursor = sky_cursor_create();
    return cursor;
}

// Frees a cursor.
//
// cursor - The cursor to free.
void sky_eql_cursor_free(sky_eql_cursor *cursor)
{
    if(cursor) {
        cursor->cursor = NULL;
        free(cursor);
    }
}


// TODO: next()
// TODO: eof()