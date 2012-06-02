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

int test_eql_ast_var_decl_create() {
    eql_ast_node *node;
    eql_ast_var_decl_create(&foo, &bar, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_DECL, "");
    mu_assert(biseqcstr(node->var_decl.type, "foo"), "");
    mu_assert(biseqcstr(node->var_decl.name, "bar"), "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_var_decl_create);
    return 0;
}

RUN_TESTS()