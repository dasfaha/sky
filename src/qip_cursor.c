#include <stdlib.h>

#include "qip_cursor.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a cursor.
sky_qip_cursor *sky_qip_cursor_create()
{
    sky_qip_cursor *cursor = malloc(sizeof(sky_qip_cursor));
    cursor->cursor = sky_cursor_create();
    return cursor;
}

// Frees a cursor.
//
// cursor - The cursor to free.
void sky_qip_cursor_free(sky_qip_cursor *cursor)
{
    if(cursor) {
        cursor->cursor = NULL;
        free(cursor);
    }
}

//--------------------------------------
// Cursor Management
//--------------------------------------

// Retrieves the next event in the cursor.
//
// module - The module.
// cursor - The cursor.
// event  - The event object to update.
//
// Returns nothing.
void sky_qip_cursor_next(qip_module *module, sky_qip_cursor *cursor,
                         sky_qip_event *event)
{
    check(module != NULL, "Module required");

    // Update the action id on the event.
    sky_action_id_t action_id;
    sky_cursor_get_action_id(cursor->cursor, &action_id);
    event->action_id = (int64_t)action_id;
    
    // TODO: Update state on event.

    // Move to the next event in the cursor.
    sky_cursor_next(cursor->cursor);
    
    return;
    
error:
    return;
}

// Checks whether the cursor is at the end.
//
// module - The module.
// cursor - The cursor.
//
// Returns a flag stating if the cursor is done.
bool sky_qip_cursor_eof(qip_module *module, sky_qip_cursor *cursor)
{
    check(module != NULL, "Module required");
    return cursor->cursor->eof;

error:
    return true;
}
