#ifndef _qip_string_h
#define _qip_string_h

#include <inttypes.h>
#include <stdbool.h>

typedef struct qip_string qip_string;

#include "module.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// The qip string stores information about a fixed length string.
struct qip_string {
    int64_t length;
    char *data;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

qip_string qip_string_create(int64_t length, char *data);


//======================================
// Equality
//======================================

bool qip_string_equals(qip_module *module, qip_string a, qip_string b);

#endif
