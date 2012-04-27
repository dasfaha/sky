#include "cursor.h"
#include "path.h"
#include "event.h"
#include "mem.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

// Calculates the length of an event stored in raw format at the given pointer.
//
// ptr - A pointer to the raw event data.
//
// Returns the length of the raw event data.
uint32_t get_event_length(void *ptr)
{
    uint32_t length = EVENT_HEADER_LENGTH;
    char event_flag = *((char*)ptr);
    
    // Add action length.
    if(event_flag & EVENT_FLAG_ACTION) {
        length += sizeof(int32_t);
    }

    // Add data length.
    if(event_flag & EVENT_FLAG_DATA) {
        void *data_length_ptr = ptr + length;
        uint16_t data_length = *((uint16_t*)data_length_ptr);
        length += sizeof(int16_t) + data_length;
    }
    
    return length;
}    


//======================================
// Lifecycle
//======================================

// Creates a reference to a cursor.
// 
// Returns a reference to the new cursor if successful. Otherwise returns
// null.
Cursor *Cursor_create()
{
    Cursor *cursor;
    
    cursor = malloc(sizeof(Cursor)); check_mem(cursor);
    cursor->paths = NULL;
    cursor->path_count = 0;
    cursor->path_index = 0;
    cursor->event_index = 0;
    cursor->ptr = NULL;
    cursor->eof = false;
    
    return cursor;
    
error:
    Cursor_destroy(cursor);
    return NULL;
}

// Removes a cursor reference from memory.
//
// cursor - The cursor to free.
void Cursor_destroy(Cursor *cursor)
{
    if(cursor) {
        if(cursor->paths) free(cursor->paths);
        free(cursor);
    }
}


//======================================
// Path Management
//======================================

// Initializes the cursor to point to a new path.
//
// cursor - The cursor to update.
// index  - The index of the path.
void set_current_path(Cursor *cursor, uint32_t index)
{
    // Calculate path length
    uint32_t events_length;
    void *ptr = cursor->paths[index];
    memcpy(&events_length, ptr+sizeof(int64_t), sizeof(events_length));
    cursor->path_length = PATH_HEADER_LENGTH + events_length;

    // Store position of first event and store position of end of path.
    cursor->ptr    = ptr + PATH_HEADER_LENGTH;
    cursor->endptr = ptr + cursor->path_length;
}


// Assigns a single path to the cursor.
// 
// cursor - The cursor.
// ptr    - A pointer to the raw data where the path starts.
//
// Returns 0 if successful, otherwise returns -1.
int Cursor_set_path(Cursor *cursor, void *ptr)
{
    int rc;
    
    check(cursor != NULL, "Cursor required");

    // If data is not null then create an array of one pointer.
    if(ptr != NULL) {
        void **ptrs = malloc(sizeof(void*));
        ptrs[0] = ptr;
        rc = Cursor_set_paths(cursor, ptrs, 1);
        check(rc == 0, "Unable to set path data to cursor");
    }
    else {
        rc = Cursor_set_paths(cursor, NULL, 0);
        check(rc == 0, "Unable to remove path data");
    }

    return 0;

error:
    return -1;
}

// Assigns a list of paths to the cursor.
// 
// cursor - The cursor.
// ptrs   - An array to pointers of raw paths.
//
// Returns 0 if successful, otherwise returns -1.
int Cursor_set_paths(Cursor *cursor, void **ptrs, int count)
{
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
    
    // Position the pointer at the first path.
    if(count > 0) {
        set_current_path(cursor, 0);
    }
    else {
        cursor->path_length = 0;
        cursor->ptr = NULL;
        cursor->endptr = NULL;
    }
    
    return 0;

error:
    return -1;
}


//======================================
// Iteration
//======================================

int Cursor_next_event(Cursor *cursor)
{
    check(cursor != NULL, "Cursor required");
    check(!cursor->eof, "No more events are available");

    // Move to next event.
    uint32_t event_length = get_event_length(cursor->ptr);
    cursor->ptr += event_length;
    cursor->event_index++;

    // If pointer is beyond the last event then move to next path.
    if(cursor->ptr >= cursor->endptr) {
        cursor->path_index++;

        // Move to the next path if more paths are remaining.
        if(cursor->path_index < cursor->path_count) {
            set_current_path(cursor, cursor->path_index);
        }
        // Otherwise set EOF.
        else {
            cursor->path_index = 0;
            cursor->event_index = 0;
            cursor->eof = true;
            cursor->path_length = 0;
            cursor->ptr    = NULL;
            cursor->endptr = NULL;
        }
    }

    return 0;

error:
    return -1;
}

