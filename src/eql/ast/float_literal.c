#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a literal floating point number.
//
// value - The floating point value.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_float_literal_create(double value, eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FLOAT_LITERAL;
    node->float_literal.value = value;
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_float_literal_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_FLOAT_LITERAL, "Node type must be 'float literal'");
    
    *type = bfromcstr("Float");
    return 0;

error:
    *type = NULL;
    return -1;
}

