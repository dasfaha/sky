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

int test_eql_ast_method_create() {
    eql_ast_node *node, *function;
    eql_ast_function_create(&foo, &bar, NULL, 0, NULL, &function);
    eql_ast_method_create(EQL_ACCESS_PUBLIC, function, &node);
    mu_assert(node->type == EQL_AST_TYPE_METHOD, "");
    mu_assert(node->method.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(node->method.function == function, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_method_create);
    return 0;
}

RUN_TESTS()