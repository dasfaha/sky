#ifndef _property_file_h
#define _property_file_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_property_file sky_property_file;

#include "bstring.h"
#include "file.h"
#include "types.h"
#include "property.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct sky_property_file {
    bstring path;
    sky_property **properties;
    uint32_t property_count;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_property_file *sky_property_file_create();

void sky_property_file_free(sky_property_file *property_file);


//--------------------------------------
// Path
//--------------------------------------

int sky_property_file_set_path(sky_property_file *property_file, bstring path);

int sky_property_file_get_path(sky_property_file *property_file, bstring *path);


//--------------------------------------
// Persistence
//--------------------------------------

int sky_property_file_load(sky_property_file *property_file);

int sky_property_file_unload(sky_property_file *property_file);

int sky_property_file_save(sky_property_file *property_file);


//--------------------------------------
// Property Management
//--------------------------------------

int sky_property_file_find_property_by_id(sky_property_file *property_file,
    sky_property_id_t property_id, sky_property **ret);

int sky_property_file_find_property_by_name(sky_property_file *property_file,
    bstring name, sky_property **ret);

int sky_property_file_add_property(sky_property_file *property_file, sky_property *ret);

#endif
