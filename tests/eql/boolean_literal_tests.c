#include <stdio.h>
#include <stdlib.h>

#include <dbg.h>
#include <eql/ast.h>
#include <eql/parser.h>
#include <eql/module.h>

#include "../minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_boolean_literal_create() {
    eql_ast_node *node;
    eql_ast_boolean_literal_create(true, &node);
    mu_assert(node->type == EQL_AST_TYPE_BOOLEAN_LITERAL, "");
    mu_assert(node->boolean_literal.value == true, "");
    mu_assert_eql_node_dump(node, "<boolean-literal value='true'>\n");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_boolean_literal_true() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("true;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BOOLEAN_LITERAL, "");
    mu_assert(node->boolean_literal.value == true, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_boolean_literal_false() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("true;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BOOLEAN_LITERAL, "");
    mu_assert(node->boolean_literal.value == true, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

//--------------------------------------
// Type
//--------------------------------------

int test_eql_boolean_literal_get_type() {
    eql_ast_node *node;
    bstring type;
    eql_module *module = eql_module_create(&foo, NULL);
    eql_ast_boolean_literal_create(true, &node);
    eql_ast_node_get_type(node, module, &type);
    mu_assert(biseqcstr(type, "Boolean"), "");
    eql_ast_node_free(node);
    eql_module_free(module);
    bdestroy(type);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_boolean_literal_true() {
    mu_assert_eql_compile("return true;", "tests/fixtures/eql/ir/boolean_literal_true.ll");
    return 0;
}

int test_eql_compile_boolean_literal_false() {
    mu_assert_eql_compile("return false;", "tests/fixtures/eql/ir/boolean_literal_false.ll");
    return 0;
}


//--------------------------------------
// Execute  
//--------------------------------------

int test_eql_execute_boolean_literal_true() {
    mu_assert_eql_execute_boolean("return true;", true);
    return 0;
}

int test_eql_execute_boolean_literal_false() {
    mu_assert_eql_execute_boolean("return false;", false);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_boolean_literal_create);
    mu_run_test(test_eql_parse_boolean_literal_true);
    mu_run_test(test_eql_parse_boolean_literal_false);
    mu_run_test(test_eql_boolean_literal_get_type);
    mu_run_test(test_eql_compile_boolean_literal_true);
    mu_run_test(test_eql_compile_boolean_literal_false);
    mu_run_test(test_eql_execute_boolean_literal_true);
    mu_run_test(test_eql_execute_boolean_literal_false);
    return 0;
}

RUN_TESTS()