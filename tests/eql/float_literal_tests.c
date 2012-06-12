#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>

#include "../minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_float_literal_create() {
    eql_ast_node *node;
    eql_ast_float_literal_create(100.293, &node);
    mu_assert(node->type == EQL_AST_TYPE_FLOAT_LITERAL, "");
    mu_assert(node->float_literal.value == 100.293, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_float_literal_parse() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("10.421;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_FLOAT_LITERAL, "");
    mu_assert(node->float_literal.value == 10.421, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Type
//--------------------------------------

int test_eql_float_literal_get_type() {
    eql_ast_node *node;
    bstring type;
    eql_ast_float_literal_create(100.1, &node);
    eql_ast_node_get_type(node, &type);
    mu_assert(biseqcstr(type, "Float"), "");
    eql_ast_node_free(node);
    bdestroy(type);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_float_literal_create);
    mu_run_test(test_eql_float_literal_parse);
    mu_run_test(test_eql_float_literal_get_type);
    return 0;
}

RUN_TESTS()