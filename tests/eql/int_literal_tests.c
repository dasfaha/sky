#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>

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
    bstring text = bfromcstr("200");
    eql_parse(&foo, text, &module);
    eql_ast_node *node = module->module.block->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->int_literal.value == 200, "");
    eql_ast_node_free(module);
    bdestroy(text);
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
    return 0;
}

RUN_TESTS()