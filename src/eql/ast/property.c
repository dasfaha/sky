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

// Creates an AST node for a property.
//
// var_decl - The variable declaration.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_property_create(eql_ast_access_e access,
                            eql_ast_node *var_decl,
                            eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_PROPERTY;
    node->parent = NULL;
    node->property.access   = access;
    node->property.var_decl = var_decl;
    if(var_decl != NULL) {
        var_decl->parent = node;
    }
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a property AST node from memory.
//
// node - The AST node to free.
void eql_ast_property_free(struct eql_ast_node *node)
{
    if(node->property.var_decl) eql_ast_node_free(node->property.var_decl);
    node->property.var_decl = NULL;
}


//--------------------------------------
// Debugging
//--------------------------------------

// Append the contents of the AST node to the string.
// 
// node - The node to dump.
// ret  - A pointer to the bstring to concatenate to.
//
// Return 0 if successful, otherwise returns -1.s
int eql_ast_property_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<property>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->property.var_decl != NULL) {
        rc = eql_ast_node_dump(node->property.var_decl, ret);
        check(rc == 0, "Unable to dump method function");
    }

    return 0;

error:
    return -1;
}
