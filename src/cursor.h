#ifndef _cursor_h
#define _cursor_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bstring.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The cursor is used to iterate over the events of a path. It provides fast
// access to events in a path by performing data access on the raw bytes of the
// data file. It also abstracts away the underlying storage of the events by
// seamlessly combining spanned blocks into a single path.
//
// The current API to the cursor is simple. It provides forward-only access to
// basic event data in a path. However, future releases will allow bidirectional
// traversal, event search, & object state management.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Cursor {
    void **paths;
    uint32_t path_count;
    uint32_t path_index;
    uint32_t event_index;
    uint32_t path_length;
    void *ptr;
    void *endptr;
    bool eof;
} Cursor;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

Cursor *Cursor_create();

void Cursor_destroy(Cursor *cursor);


//======================================
// Event Management
//======================================

int Cursor_get_action(Cursor *cursor, int32_t *action_id);


//======================================
// Path Management
//======================================

int Cursor_set_path(Cursor *cursor, void *ptr);

int Cursor_set_paths(Cursor *cursor, void **ptrs, int count);


//======================================
// Iteration
//======================================

int Cursor_next_event(Cursor *cursor);


#endif
