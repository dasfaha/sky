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

// Creates an AST node for a variable reference.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_ref_create(bstring name, eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_VAR_REF;
    node->parent = NULL;
    node->var_ref.name = bstrcpy(name);
    check_mem(node->var_ref.name);
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void eql_ast_var_ref_free(eql_ast_node *node)
{
    if(node->var_ref.name) {
        bdestroy(node->var_ref.name);
    }
    node->var_ref.name = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the variable reference AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_ref_codegen(eql_ast_node *node, eql_module *module,
                            LLVMValueRef *value)
{
    int rc;

	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_VAR_REF, "Node type expected to be 'variable reference'");
	check(module != NULL, "Module required");
	check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

	// Find the variable declaration.
	eql_ast_node *var_decl;
	LLVMValueRef var_decl_value;
	rc = eql_module_get_variable(module, node->var_ref.name, &var_decl, &var_decl_value);
	check(rc == 0, "Unable to retrieve variable declaration: %s", bdata(node->var_ref.name));
	check(var_decl != NULL, "No variable declaration found: %s", bdata(node->var_ref.name));
	check(var_decl_value != NULL, "No LLVM value for variable declaration: %s", bdata(node->var_ref.name));

	// Create load instruction.
	*value = LLVMBuildLoad(builder, var_decl_value, bdata(node->var_ref.name));
	check(*value != NULL, "Unable to create load instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_var_ref_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_VAR_REF, "Node type must be 'var ref'");

	// Search up the parent hierarchy to find variable declaration.
	eql_ast_node *var_decl = NULL;
	eql_ast_node *parent = node->parent;
	while(parent != NULL) {
		int rc = eql_ast_node_get_var_decl(parent, node->var_ref.name, &var_decl);
		check(rc == 0, "Unable to search node for variable declarations");
		
		// If a declaration was found then return its type.
		if(var_decl != NULL) {
            *type = var_decl->var_decl.type;
			return 0;
		}
		
		parent = parent->parent;
	}

	sentinel("Unable to find variable declaration: %s", bdata(node->var_ref.name));

error:
    *type = NULL;
    return -1;
}
