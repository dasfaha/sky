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

int test_eql_ast_int_literal_create() {
    eql_ast_node *node;
    eql_ast_int_literal_create(20, &node);
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->int_literal.value == 20, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_int_literal() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("200;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->int_literal.value == 200, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

//--------------------------------------
// Type
//--------------------------------------

int test_eql_int_literal_get_type() {
    eql_ast_node *node;
    bstring type;
    eql_ast_int_literal_create(20, &node);
    eql_ast_node_get_type(node, &type);
    mu_assert(biseqcstr(type, "Int"), "");
    eql_ast_node_free(node);
    bdestroy(type);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_int_literal() {
    mu_assert_eql_compile("return 200;", "tests/fixtures/eql/ir/int_literal.ll");
    return 0;
}


//--------------------------------------
// Execute  
//--------------------------------------

int test_eql_execute_int_literal() {
    mu_assert_eql_execute_int("return 200;", 200);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_int_literal_create);
    mu_run_test(test_eql_parse_int_literal);
    mu_run_test(test_eql_int_literal_get_type);
    mu_run_test(test_eql_compile_int_literal);
    mu_run_test(test_eql_execute_int_literal);
    return 0;
}

RUN_TESTS()