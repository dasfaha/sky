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

int test_eql_ast_property_create() {
    eql_ast_node *node, *var_decl;
    eql_ast_var_decl_create(&foo, &bar, &var_decl);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, var_decl, &node);
    mu_assert(node->type == EQL_AST_TYPE_PROPERTY, "");
    mu_assert(node->property.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(node->property.var_decl == var_decl, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_property_create);
    return 0;
}

RUN_TESTS()