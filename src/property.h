#ifndef _sky_property_h
#define _sky_property_h

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct sky_property sky_property;

#include "bstring.h"
#include "file.h"
#include "property_file.h"

//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef enum {
    SKY_PROPERTY_TYPE_OBJECT = 1,
    SKY_PROPERTY_TYPE_ACTION = 2,
} sky_property_type_e;

struct sky_property {
    sky_property_file *property_file;
    sky_property_id_t id;
    sky_property_type_e type;
    bstring data_type;
    bstring name;
};



//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_property *sky_property_create();

void sky_property_free(sky_property *property);

//--------------------------------------
// Serialization
//--------------------------------------

size_t sky_property_sizeof(sky_property *property);

int sky_property_pack(sky_property *property, FILE *file);

int sky_property_unpack(sky_property *property, FILE *file);

#endif
