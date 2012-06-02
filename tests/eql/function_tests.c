#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>

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

int test_eql_ast_function_create() {
    eql_ast_node *node, *prototype, *body;
    eql_ast_fproto_create(&foo, &bar, NULL, 0, &prototype);
    eql_ast_int_literal_create(10, &body);
    eql_ast_function_create(prototype, body, &node);
    mu_assert(node->type == EQL_AST_TYPE_FUNCTION, "");
    mu_assert(node->function.prototype == prototype, "");
    mu_assert(node->function.body == body, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_function_create);
    return 0;
}

RUN_TESTS()