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

// Creates an AST node for a variable declaration.
//
// type  - The type of variable that is being defined.
// name  - The name of the variable being defined.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_decl_create(bstring type, bstring name,
                            struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_VAR_DECL;
    node->parent = NULL;
    node->var_decl.type = bstrcpy(type);
    check_mem(node->var_decl.type);
    node->var_decl.name = bstrcpy(name);
    check_mem(node->var_decl.name);
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
void eql_ast_var_decl_free(struct eql_ast_node *node)
{
    if(node->var_decl.type) {
        bdestroy(node->var_decl.type);
    }
    node->var_decl.type = NULL;

    if(node->var_decl.name) {
        bdestroy(node->var_decl.name);
    }
    node->var_decl.name = NULL;
}


