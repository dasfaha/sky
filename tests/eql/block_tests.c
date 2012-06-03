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

int test_eql_ast_block_create() {
    eql_ast_node *exprs[2];
    eql_ast_node *node, *expr1, *expr2;
    eql_ast_int_literal_create(10, &expr1);
    eql_ast_int_literal_create(20, &expr2);
    exprs[0] = expr1;
    exprs[1] = expr2;
    eql_ast_block_create(exprs, 2, &node);
    mu_assert(node->type == EQL_AST_TYPE_BLOCK, "");
    mu_assert(node->block.expr_count == 2, "");
    mu_assert(node->block.exprs[0] == expr1, "");
    mu_assert(node->block.exprs[1] == expr2, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_block() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("2 + 1; 3 - 4; 5;");
    eql_parse(NULL, text, &module);
    eql_ast_node *block = module->module.block;
    mu_assert(block->type == EQL_AST_TYPE_BLOCK, "");
    mu_assert(block->block.expr_count == 3, "");
    mu_assert(block->block.exprs[0]->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(block->block.exprs[1]->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(block->block.exprs[2]->type == EQL_AST_TYPE_INT_LITERAL, "");
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
    mu_run_test(test_eql_ast_block_create);
    mu_run_test(test_eql_parse_block);
    return 0;
}

RUN_TESTS()