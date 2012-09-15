#ifndef _sky_qip_module_h
#define _sky_qip_module_h

#include <inttypes.h>

#include "table.h"
#include "qip/qip.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// This struct wraps the Qip module to provide some additional information
// around dynamic Event properties.
typedef struct {
    qip_module *_qip_module;
    qip_compiler *compiler;
    void *main_function;
    sky_table *table;
    int64_t event_property_count;
    sky_property_id_t *event_property_ids;
    int64_t *event_property_offsets;
    bstring *event_property_types;
} sky_qip_module;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

sky_qip_module *sky_qip_module_create();

void sky_qip_module_free(sky_qip_module *module);

void sky_qip_module_free_event_info(sky_qip_module *module);

//--------------------------------------
// Compilation
//--------------------------------------

int sky_qip_module_compile(sky_qip_module *module, bstring query_text);

#endif
