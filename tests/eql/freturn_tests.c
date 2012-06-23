#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>
#include <eql/compiler.h>

#include "../minunit.h"


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_freturn_create() {
    eql_ast_node *node, *value;
    eql_ast_int_literal_create(1, &value);
    eql_ast_freturn_create(value, &node);
    mu_assert(node->type == EQL_AST_TYPE_FRETURN, "");
    mu_assert(node->freturn.value == value, "");
    mu_assert(node->freturn.value->parent == node, "");
    mu_assert_eql_node_dump(node,
        "<freturn>\n"
        "  <int-literal value='1'>\n"
    );
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_freturn() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("return 200;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_FRETURN, "");
    mu_assert(node->freturn.value->type == EQL_AST_TYPE_INT_LITERAL, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_freturn_void() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("return;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_FRETURN, "");
    mu_assert(node->freturn.value == NULL, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_freturn_void() {
    mu_assert_eql_compile("return;", "tests/fixtures/eql/ir/freturn_void.ll");
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_freturn_create);
    mu_run_test(test_eql_parse_freturn);
    mu_run_test(test_eql_parse_freturn_void);
    mu_run_test(test_eql_compile_freturn_void);
    return 0;
}

RUN_TESTS()