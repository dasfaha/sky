#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <eql/ast.h>
#include <mem.h>

#include "../minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Int Literal
//--------------------------------------

int test_eql_ast_int_literal_create() {
    eql_ast_node *node;
    eql_ast_int_literal_create(20, &node);
    mu_assert(node->int_literal.value == 20, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Float Literal
//--------------------------------------

int test_eql_ast_float_literal_create() {
    eql_ast_node *node;
    eql_ast_float_literal_create(100.293, &node);
    mu_assert(node->float_literal.value == 100.293, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Binary Expression
//--------------------------------------

int test_eql_ast_binary_expr_create() {
    eql_ast_node *node, *lhs, *rhs;
    eql_ast_int_literal_create(10, &lhs);
    eql_ast_float_literal_create(10, &rhs);
    eql_ast_binary_expr_create(EQL_BINOP_MUL, lhs, rhs, &node);
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs == lhs, "");
    mu_assert(node->binary_expr.rhs == rhs, "");
    eql_ast_node_free(node);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_int_literal_create);
    mu_run_test(test_eql_ast_float_literal_create);
    mu_run_test(test_eql_ast_binary_expr_create);
    return 0;
}

RUN_TESTS()