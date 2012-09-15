#ifndef _sky_qip_path_h
#define _sky_qip_path_h

#include <inttypes.h>

#include "path_iterator.h"
#include "qip_cursor.h"
#include "qip/qip.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// The path stores a reference to the current path.
typedef struct {
    void *path_ptr;
} sky_qip_path;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_qip_path *sky_qip_path_create();

void sky_qip_path_free(sky_qip_path *path);


//--------------------------------------
// Cursor Management
//--------------------------------------

sky_qip_cursor *sky_qip_path_events(qip_module *module, sky_qip_path *path);

#endif
