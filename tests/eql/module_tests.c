#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <eql/module.h>

#include "../minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring Int = bsStatic("Int");
struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Scopes
//--------------------------------------

int test_eql_module_scopes() {
	int rc;
	eql_ast_node *node;
    eql_ast_function_create(NULL, NULL, NULL, 0, NULL, &node);
	eql_compiler *compiler = eql_compiler_create();
	eql_module *module = eql_module_create(&foo, compiler);
	rc = eql_module_push_scope(module, node);
	mu_assert(rc == 0, "");
    mu_assert(module->scope_count == 1, "");
    mu_assert(module->scopes[0]->node == node, "");
	rc = eql_module_pop_scope(module, node);
	mu_assert(rc == 0, "");
    mu_assert(module->scope_count == 0, "");
    eql_compiler_free(compiler);
    eql_module_free(module);
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Scope Variabels
//--------------------------------------

int test_eql_module_scope_variables() {
	int rc;
	eql_ast_node *func, *block1, *block2;
	eql_ast_node *var_decl1, *var_decl2, *var_decl3; 

    eql_ast_function_create(NULL, NULL, NULL, 0, NULL, &func);
    eql_ast_block_create(NULL, NULL, 0, &block1);
    eql_ast_block_create(NULL, NULL, 0, &block2);

    eql_ast_var_decl_create(&Int, &foo, NULL, &var_decl1);
	LLVMValueRef value1 = LLVMConstInt(LLVMInt64Type(), 10, true);
    eql_ast_var_decl_create(&Int, &bar, NULL, &var_decl2);
	LLVMValueRef value2 = LLVMConstInt(LLVMInt64Type(), 20, true);
    eql_ast_var_decl_create(&Int, &foo, NULL, &var_decl3);
	LLVMValueRef value3 = LLVMConstInt(LLVMInt64Type(), 30, true);

	eql_compiler *compiler = eql_compiler_create();
	eql_module *module = eql_module_create(&foo, compiler);

	// Add variables to different scopes.
	eql_module_push_scope(module, func);
	eql_module_add_variable(module, var_decl1, value1);
	eql_module_add_variable(module, var_decl2, value2);
	eql_module_push_scope(module, block1);
	eql_module_push_scope(module, block2);
	eql_module_add_variable(module, var_decl3, value3);

	// Verify that we retrieve the appropriate variables.
	eql_ast_node *ret_var_decl;
	LLVMValueRef ret_value;
	rc = eql_module_get_variable(module, &foo, &ret_var_decl, &ret_value);
	mu_assert(rc == 0, "");
	mu_assert(ret_var_decl == var_decl3, "");
	mu_assert(ret_value == value3, "");
	rc = eql_module_get_variable(module, &bar, &ret_var_decl, &ret_value);
	mu_assert(rc == 0, "");
	mu_assert(ret_var_decl == var_decl2, "");
	mu_assert(ret_value == value2, "");

	// Move back up the stack and try again.
	eql_module_pop_scope(module, block2);
	rc = eql_module_get_variable(module, &foo, &ret_var_decl, &ret_value);
	mu_assert(rc == 0, "");
	mu_assert(ret_var_decl == var_decl1, "");
	mu_assert(ret_value == value1, "");

	rc = eql_module_get_variable(module, &baz, &ret_var_decl, &ret_value);
	mu_assert(rc == 0, "");
	mu_assert(ret_var_decl == NULL, "");
	mu_assert(ret_value == NULL, "");

    eql_compiler_free(compiler);
    eql_module_free(module);
    eql_ast_node_free(var_decl1);
    eql_ast_node_free(var_decl2);
    eql_ast_node_free(var_decl3);

    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_module_scopes);
    mu_run_test(test_eql_module_scope_variables);
    return 0;
}

RUN_TESTS()