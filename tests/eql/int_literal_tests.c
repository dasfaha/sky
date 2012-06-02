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

int test_eql_ast_int_literal_create() {
    eql_ast_node *node;
    eql_ast_int_literal_create(20, &node);
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->int_literal.value == 20, "");
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
    return 0;
}

RUN_TESTS()