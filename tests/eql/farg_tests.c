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

int test_eql_ast_farg_create() {
    eql_ast_node *node, *var_decl;
    eql_ast_var_decl_create(&foo, &bar, NULL, &var_decl);
    eql_ast_farg_create(var_decl, &node);
    mu_assert(node->type == EQL_AST_TYPE_FARG, "");
    mu_assert(node->farg.var_decl == var_decl, "");
    mu_assert(node->farg.var_decl->parent == node, "");
    mu_assert_eql_node_dump(node,
        "<farg>\n"
        "  <var-decl type='foo' name='bar'>\n"
    );
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_farg_create);
    return 0;
}

RUN_TESTS()