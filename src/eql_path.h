#ifndef _sky_eql_path_h
#define _sky_eql_path_h

#include <inttypes.h>

#include "path_iterator.h"
#include "eql_cursor.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// The path stores a reference to the current path.
typedef struct {
    void *path_ptr;
} sky_eql_path;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_eql_path *sky_eql_path_create();

void sky_eql_path_free(sky_eql_path *path);


//======================================
// Cursor Management
//======================================

sky_eql_cursor *sky_eql_path_events(sky_eql_path *path);

#endif
