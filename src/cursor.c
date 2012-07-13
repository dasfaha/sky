#include "cursor.h"
#include "path.h"
#include "event.h"
#include "mem.h"
#include "dbg.h"
#include "minipack.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_cursor_set_ptr(sky_cursor *cursor, void *ptr);
int sky_cursor_set_eof(sky_cursor *cursor);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to a cursor.
// 
// Returns a reference to the new cursor if successful.
sky_cursor *sky_cursor_create()
{
    sky_cursor *cursor = calloc(sizeof(sky_cursor),1 ); check_mem(cursor);
    return cursor;
    
error:
    sky_cursor_free(cursor);
    return NULL;
}

// Removes a cursor reference from memory.
//
// cursor - The cursor to free.
void sky_cursor_free(sky_cursor *cursor)
{
    if(cursor) {
        if(cursor->paths) free(cursor->paths);
        free(cursor);
    }
}


//======================================
// Path Management
//======================================

// Assigns a single path to the cursor.
// 
// cursor - The cursor.
// ptr    - A pointer to the raw data where the path starts.
//
// Returns 0 if successful, otherwise returns -1.
int sky_cursor_set_path(sky_cursor *cursor, void *ptr)
{
    int rc;
    check(cursor != NULL, "Cursor required");

    // If data is not null then create an array of one pointer.
    void **ptrs;
    if(ptr != NULL) {
        ptrs = malloc(sizeof(void*)); check_mem(ptrs);
        ptrs[0] = ptr;
        rc = sky_cursor_set_paths(cursor, ptrs, 1);
        check(rc == 0, "Unable to set path data to cursor");
    }
    // Otherwise clear out pointer paths.
    else {
        rc = sky_cursor_set_paths(cursor, NULL, 0);
        check(rc == 0, "Unable to remove path data");
    }

    return 0;

error:
    if(ptrs) free(ptrs);
    return -1;
}

// Assigns a list of path pointers to the cursor.
// 
// cursor - The cursor.
// ptrs   - An array to pointers of raw paths.
//
// Returns 0 if successful, otherwise returns -1.
int sky_cursor_set_paths(sky_cursor *cursor, void **ptrs, int count)
{
    int rc;
    check(cursor != NULL, "Cursor required");
    
    // Free old path list.
    if(cursor->paths != NULL) {
        free(cursor->paths);
    }

    // Assign path data list.
    cursor->paths = ptrs;
    cursor->path_count = count;
    cursor->path_index = 0;
    cursor->event_index = 0;
    cursor->eof = (count == 0);
    
    // Position the pointer at the first path if paths are passed.
    if(count > 0) {
        rc = sky_cursor_set_ptr(cursor, cursor->paths[0]);
        check(rc == 0, "Unable to set paths");
    }
    // Otherwise clear out the pointer.
    else {
        cursor->ptr = NULL;
        cursor->endptr = NULL;
    }
    
    return 0;

error:
    return -1;
}


//======================================
// Pointer Management
//======================================

// Initializes the cursor to point at a new path at a given pointer.
//
// cursor - The cursor to update.
// ptr    - The address of the start of a path.
//
// Returns 0 if successful, otherwise returns -1.
int sky_cursor_set_ptr(sky_cursor *cursor, void *ptr)
{
    size_t sz;
    
    // Skip over header.
    sz = sky_path_sizeof_raw_hdr(ptr);
    check(sz > 0, "Unable to determine path header size at %p", ptr);
    ptr += sz;

    // Read how long the event data is.
    uint32_t events_length = minipack_unpack_raw(ptr, &sz);
    check(sz > 0, "Unable to determine path event data length at %p", ptr);
    ptr += sz;

    // Store position of first event and store position of end of path.
    cursor->ptr    = ptr;
    cursor->endptr = cursor->ptr + events_length;
    
    return 0;

error:
    cursor->ptr = NULL;
    cursor->endptr = NULL;
    return -1;
}


//======================================
// Iteration
//======================================

int sky_cursor_next(sky_cursor *cursor)
{
    int rc;
    check(cursor != NULL, "Cursor required");
    check(!cursor->eof, "No more events are available");

    // Move to next event.
    size_t event_length = sky_event_sizeof_raw(cursor->ptr);
    cursor->ptr += event_length;
    cursor->event_index++;

    // If pointer is beyond the last event then move to next path.
    if(cursor->ptr >= cursor->endptr) {
        cursor->path_index++;

        // Move to the next path if more paths are remaining.
        if(cursor->path_index < cursor->path_count) {
            rc = sky_cursor_set_ptr(cursor, cursor->paths[cursor->path_index]);
            check(rc == 0, "Unable to set pointer to path");
        }
        // Otherwise set EOF.
        else {
            rc = sky_cursor_set_eof(cursor);
            check(rc == 0, "Unable to set EOF on cursor");
        }
    }

    return 0;

error:
    return -1;
}

// Flags a cursor to say that it is at the end of all its paths.
//
// cursor - The cursor to set EOF on.
//
// Returns 0 if successful, otherwise returns -1.
int sky_cursor_set_eof(sky_cursor *cursor)
{
    check(cursor != NULL, "Cursor required");
    
    cursor->path_index  = 0;
    cursor->event_index = 0;
    cursor->eof         = true;
    cursor->ptr         = NULL;
    cursor->endptr      = NULL;

    return 0;

error:
    return -1;
}
