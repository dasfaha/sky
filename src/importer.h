#ifndef _sky_importer_h
#define _sky_importer_h

#include <stddef.h>
#include <inttypes.h>
#include <stdio.h>

#include "bstring.h"
#include "table.h"
#include "types.h"

//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct {
    bstring path;
    sky_table *table;
    sky_event **events;
} sky_importer;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_importer *sky_importer_create();

void sky_importer_free(sky_importer *importer);

//--------------------------------------
// Path Management
//--------------------------------------

int sky_importer_set_path(sky_importer *importer, bstring path);

//--------------------------------------
// Import
//--------------------------------------

int sky_importer_import(sky_importer *importer, FILE *file);

#endif
