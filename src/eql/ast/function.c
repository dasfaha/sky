#include <stdlib.h>
#include "../../dbg.h"

#include "function.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a function.
//
// prototype - The function prototype.
// body      - The contents of the function.
// ret       - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_function_create(struct eql_ast_node *prototype,
                            struct eql_ast_node *body,
                            struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FUNCTION;
    node->function.prototype = prototype;
    node->function.body      = body;
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a variable declaration AST node from memory.
//
// node - The AST node to free.
void eql_ast_function_free(struct eql_ast_node *node)
{
    if(node->function.prototype) eql_ast_node_free(node->function.prototype);
    node->function.prototype = NULL;

    if(node->function.body) eql_ast_node_free(node->function.body);
    node->function.body = NULL;
}
