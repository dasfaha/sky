#include <stdlib.h>
#include "../../dbg.h"

#include "method.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a method.
//
// function - The function.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_create(eql_ast_access_e access,
                          eql_ast_node *function,
                          eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METHOD;
    node->parent = NULL;
    node->method.access = access;
    node->method.function = function;
    if(function != NULL) {
        function->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a method AST node from memory.
//
// node - The AST node to free.
void eql_ast_method_free(struct eql_ast_node *node)
{
    if(node->method.function) eql_ast_node_free(node->method.function);
    node->method.function = NULL;
}
