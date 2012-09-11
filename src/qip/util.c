#include <stdlib.h>

#include "dbg.h"
#include "util.h"
#include "mem.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Types
//--------------------------------------

// Retrieves a flag stating if the type is a built-in data type.
//
// name - The name of the type.
//
// Returns true if the the type is a built-in, otherwise returns false.
bool qip_is_builtin_type(qip_ast_node *node)
{
    // If this is a function type then it is built-in.
    if(qip_is_function_type(node)) {
        return true;
    }
    // If this type ref has no subtypes then check the name.
    else if(node != NULL && node->type == QIP_AST_TYPE_TYPE_REF && node->type_ref.subtype_count == 0) {
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
    // If this is a function type then it is built-in.
    if(qip_is_function_type_name(name)) {
        return true;
    }
    else {
        return biseqcstr(name, "Int") == 1
            || biseqcstr(name, "Float") == 1
            || biseqcstr(name, "Boolean") == 1
            || biseqcstr(name, "Ref") == 1
            || biseqcstr(name, "Function") == 1
            || biseqcstr(name, "void") == 1;
    }
}

// Retrieves a flag stating if the type is a function type.
//
// node - The type ref node.
//
// Returns true if the the type is a function type, otherwise returns false.
bool qip_is_function_type(qip_ast_node *node)
{
    return node != NULL
        && node->type == QIP_AST_TYPE_TYPE_REF
        && biseqcstr(node->type_ref.name, "Function") == 1;
}

// Retrieves a flag stating if the type name is a function type.
//
// name - The name of the type.
//
// Returns true if the the type is a function type, otherwise returns false.
bool qip_is_function_type_name(bstring name)
{
    struct tagbstring function_str1 = bsStatic("Function");
    struct tagbstring function_str2 = bsStatic("Function<");
    
    return binstr(name, 0, &function_str1) != BSTR_ERR
        || binstr(name, 0, &function_str2) != BSTR_ERR;
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

//--------------------------------------
// Types
//--------------------------------------

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
