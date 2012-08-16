#ifndef _eql_util_h
#define _eql_util_h

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

bool eql_is_builtin_type(eql_ast_node *node);

bool eql_is_builtin_type_name(bstring name);


//======================================
// Logging
//======================================

void eql_log(void *obj, int64_t value);

#endif
