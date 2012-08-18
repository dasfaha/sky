#ifndef _qip_util_h
#define _qip_util_h

#include <stdbool.h>

#include "bstring.h"
#include "node.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Types
//======================================

bool qip_is_builtin_type(qip_ast_node *node);

bool qip_is_builtin_type_name(bstring name);


//======================================
// Logging
//======================================

void qip_log(void *obj, int64_t value);

#endif
