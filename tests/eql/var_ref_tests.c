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


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_var_ref_create() {
    eql_ast_node *node;
    eql_ast_var_ref_create(&foo, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_REF, "");
    mu_assert(biseqcstr(node->var_ref.name, "foo"), "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_var_ref_create);
    return 0;
}

RUN_TESTS()