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
    node->parent = NULL;
    node->farg.var_decl = var_decl;
    if(var_decl != NULL) var_decl->parent = node;
    
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

// Recursively generates LLVM code for the function argument AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_farg_codegen(eql_ast_node *node, eql_module *module,
                         LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_FARG, "Node type must be 'function argument'");
    check(node->farg.var_decl != NULL, "Function argument declaration required");
    check(module != NULL, "Module required");

    // Delegate LLVM generation to the variable declaration.
    rc = eql_ast_node_codegen(node->farg.var_decl, module, value);
    check(rc == 0, "Unable to codegen function argument");
    
    return 0;

error:
    return -1;
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
int eql_ast_farg_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<farg>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->farg.var_decl != NULL) {
        rc = eql_ast_node_dump(node->farg.var_decl, ret);
        check(rc == 0, "Unable to dump variable declaration");
    }

    return 0;

error:
    return -1;
}
