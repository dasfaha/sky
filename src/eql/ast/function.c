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
// name        - The name of the function.
// return_type - The data type that the function returns.
// args        - The arguments of the function.
// arg_count   - The number of arguments the function has.
// body        - The contents of the function.
// ret         - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_function_create(bstring name, bstring return_type,
                            struct eql_ast_node **args, unsigned int arg_count,
                            struct eql_ast_node *body,
                            struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FUNCTION;
    node->function.name = bstrcpy(name);
    check_mem(node->function.name);
    node->function.return_type = bstrcpy(return_type);
    check_mem(node->function.return_type);

    // Copy arguments.
    if(arg_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * arg_count;
        node->function.args = malloc(sz);
        check_mem(node->function.args);
        memcpy(node->function.args, args, sz);
    }
    else {
        node->function.args = NULL;
    }
    node->function.arg_count = arg_count;
    
    // Assign function body.
    node->function.body = body;

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
    if(node->function.name) bdestroy(node->function.name);
    node->function.name = NULL;

    if(node->function.return_type) bdestroy(node->function.return_type);
    node->function.return_type = NULL;
    
    if(node->function.arg_count > 0) {
        unsigned int i;
        for(i=0; i<node->function.arg_count; i++) {
            eql_ast_node_free(node->function.args[i]);
            node->function.args[i] = NULL;
        }
        free(node->function.args);
        node->function.arg_count = 0;
    }

    if(node->function.body) eql_ast_node_free(node->function.body);
    node->function.body = NULL;
}
