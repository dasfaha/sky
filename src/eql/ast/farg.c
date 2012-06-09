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

// Creates an AST node for a function argument declaration.
//
// var_decl - The variable declaration.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_farg_create(struct eql_ast_node *var_decl, struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FARG;
    node->farg.var_decl = var_decl;
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
void eql_ast_farg_free(struct eql_ast_node *node)
{
    if(node->farg.var_decl) {
        eql_ast_node_free(node->farg.var_decl);
    }
    node->farg.var_decl = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_farg_typegen(eql_ast_node *node, eql_module *module,
                         LLVMTypeRef *type)
{
    check(node != NULL, "Node is required");
    check(node->type == EQL_AST_TYPE_FARG, "Node must be a function argument");
    
    int rc = eql_ast_var_decl_typegen(node->farg.var_decl, module, type);
    check(rc == 0, "Unable to generate type for function argument");

    return 0;

error:
    return -1;
}
