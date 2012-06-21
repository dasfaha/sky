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

// Creates an AST node for a variable assignment.
//
// var_ref - The variable to assign the expression to.
// expr    - The expression to assign to the variable.
// ret     - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_assign_create(eql_ast_node *var_ref,
                              eql_ast_node *expr,
                              eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_VAR_ASSIGN;
    node->parent = NULL;

    node->var_assign.var_ref = var_ref;
    if(var_ref != NULL) {
        var_ref->parent = node;
    }

    node->var_assign.expr = expr;
    if(expr != NULL) {
        expr->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a binary expression AST node from memory.
//
// node - The AST node to free.
void eql_ast_var_assign_free(eql_ast_node *node)
{
    if(node->var_assign.var_ref) eql_ast_node_free(node->var_assign.var_ref);
    node->var_assign.var_ref = NULL;

    if(node->var_assign.expr) eql_ast_node_free(node->var_assign.expr);
    node->var_assign.expr = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the variable assignment AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_assign_codegen(eql_ast_node *node,
                                eql_module *module,
                                LLVMValueRef *value)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_VAR_ASSIGN, "Node type must be 'variable assignment'");
    check(module != NULL, "Module required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

	// Generate expression.
    LLVMValueRef expr;
    rc = eql_ast_node_codegen(node->var_assign.expr, module, &expr);
    check(rc == 0 && expr != NULL, "Unable to codegen variable assignment expression");
	
	// Find the variable declaration.
    LLVMValueRef ptr = NULL;
    rc = eql_ast_node_get_var_pointer(node->var_assign.var_ref, module, &ptr);
    check(rc == 0 && ptr != NULL, "Unable to retrieve variable reference pointer");

	// Create a store instruction.
	*value = LLVMBuildStore(builder, expr, ptr);
	check(*value != NULL, "Unable to generate store instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}
