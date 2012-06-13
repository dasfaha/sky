#include <stdlib.h>
#include "../../dbg.h"

#include "fcall.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a function call.
//
// name      - The name of the function that is being called.
// args      - An array of argument values passed to the function.
// arg_count - The number of arguments passed to the function.
// ret       - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_fcall_create(bstring name, struct eql_ast_node **args,
                         unsigned int arg_count, struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FCALL;
    node->parent = NULL;
    node->fcall.name = bstrcpy(name); check_mem(node->fcall.name);

    // Copy arguments.
    if(arg_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * arg_count;
        node->fcall.args = malloc(sz);
        check_mem(node->fcall.args);
        
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->fcall.args[i] = args[i];
            args[i]->parent = node;
        }
    }
    else {
        node->fcall.args = NULL;
    }
    node->fcall.arg_count = arg_count;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void eql_ast_fcall_free(struct eql_ast_node *node)
{
    if(node->fcall.name) bdestroy(node->fcall.name);
    node->fcall.name = NULL;
    
    if(node->fcall.arg_count > 0) {
        unsigned int i;
        for(i=0; i<node->fcall.arg_count; i++) {
            eql_ast_node_free(node->fcall.args[i]);
            node->fcall.args[i] = NULL;
        }
        free(node->fcall.args);
        node->fcall.arg_count = 0;
    }
}
