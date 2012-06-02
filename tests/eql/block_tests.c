#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>

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


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_block_create);
    return 0;
}

RUN_TESTS()