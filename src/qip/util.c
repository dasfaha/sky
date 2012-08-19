#include <stdlib.h>

#include "dbg.h"
#include "util.h"
#include "mem.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Types
//======================================

// Retrieves a flag stating if the type is a built-in data type.
//
// name - The name of the type.
//
// Returns true if the the type is a built-in, otherwise returns false.
bool qip_is_builtin_type(qip_ast_node *node)
{
    // If this type ref has no subtypes then check the name.
    if(node != NULL && node->type == QIP_AST_TYPE_TYPE_REF && node->type_ref.subtype_count == 0) {
        return qip_is_builtin_type_name(node->type_ref.name);
    }
    else {
        return false;
    }
}

// Retrieves a flag stating if the name of a type is a built-in data type.
//
// name - The name of the type.
//
// Returns true if the the type is a built-in, otherwise returns false.
bool qip_is_builtin_type_name(bstring name)
{
    return biseqcstr(name, "Int")
        || biseqcstr(name, "Float")
        || biseqcstr(name, "Boolean")
        || biseqcstr(name, "Ref")
        || biseqcstr(name, "void");
}

// Retrieves a flag stating if the type is serializable.
//
// type_ref - The type reference.
//
// Returns true if the the type is serializable, otherwise returns false.
bool qip_is_serializable_type(qip_ast_node *node)
{
    // If this is a built-in and the type is Int or Float.
    if(qip_is_builtin_type(node)) {
        bstring name = node->type_ref.name;
        if(biseqcstr(name, "Int") || biseqcstr(name, "Float")) {
            return true;
        }
    }

    return false;
}

//======================================
// Types
//======================================

// A really simple log function for use with external functions.
//
// obj - The object being passed in.
// obj - The object being passed in.
//
// Returns nothing.
void qip_log(void *obj, int64_t value)
{
    debug("LOG [%p] %lld", obj, value);
    //memdump(obj, 32);
}
