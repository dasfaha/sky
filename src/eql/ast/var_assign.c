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
// name - The name of the variable to assign.
// expr - The expression to assign to the variable.
// ret  - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_assign_create(bstring name,
                              eql_ast_node *expr,
                              eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_VAR_ASSIGN;
    node->parent = NULL;
    node->var_assign.name = bstrcpy(name);
    if(name != NULL) check_mem(node->var_assign.name);

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
    if(node->var_assign.name) {
        bdestroy(node->var_assign.name);
    }
    node->var_assign.name = NULL;

    if(node->var_assign.expr) {
        eql_ast_node_free(node->var_assign.expr);
    }
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
	
	// Lookup variable in scope.
	eql_ast_node *var_decl;
    LLVMValueRef var_decl_value;
	rc = eql_module_get_variable(module, node->var_assign.name, &var_decl, &var_decl_value);
	check(rc == 0, "Unable to retrieve variable: %s", bdata(node->var_assign.name));
	check(var_decl != NULL && var_decl_value != NULL, "Variable declaration is incomplete: %s", bdata(node->var_assign.name));
	
	// Create a store instruction.
	*value = LLVMBuildStore(builder, expr, var_decl_value);
	check(*value != NULL, "Unable to generate store instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}
