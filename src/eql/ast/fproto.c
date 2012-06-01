#include <stdlib.h>
#include "../../dbg.h"

#include "fproto.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a function prototype.
//
// var_decl - The variable declaration.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_fproto_create(bstring name, bstring return_type,
                          struct eql_ast_node **args, unsigned int arg_count,
                          struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FPROTO;
    node->fproto.name = bstrcpy(name); check_mem(node->fproto.name);
    node->fproto.return_type = bstrcpy(return_type);
    check_mem(node->fproto.return_type);

    // Copy arguments.
    size_t sz = malloc(sizeof(eql_ast_node*) * arg_count);
    node->fproto.args = malloc(sz);
    check_mem(node->fproto.args);
    memcpy(node->fproto.args, args, sz);
    node->fproto.arg_count = arg_count;
    
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
void eql_ast_fproto_free(struct eql_ast_node *node)
{
    if(node->fproto.name) bdestroy(node->fproto.name);
    node->fproto.name = NULL;

    if(node->fproto.return_type) bdestroy(node->fproto.return_type);
    node->fproto.return_type = NULL;
    
    if(node->fproto.arg_count > 0) {
        int i;
        for(i=0; i<node->fproto.arg_count; i++) {
            eql_ast_node_free(node->fproto.args[i]);
            node->fproto.args[i] = NULL;
        }
        free(node->fproto.args);
        node->fproto.arg_count = 0;
    }
}
