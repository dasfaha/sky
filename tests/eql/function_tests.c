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
    eql_ast_node *args[2];
    eql_ast_node *node, *farg1, *var_decl1, *farg2, *var_decl2, *body;
    eql_ast_var_decl_create(&foo, &bar, &var_decl1);
    eql_ast_farg_create(var_decl1, &farg1);
    eql_ast_var_decl_create(&foo, &bar, &var_decl2);
    eql_ast_farg_create(var_decl2, &farg2);
    args[0] = farg1;
    args[1] = farg2;
    eql_ast_block_create(NULL, 0, &body);
    eql_ast_function_create(&foo, &bar, args, 2, body, &node);

    mu_assert(node->type == EQL_AST_TYPE_FUNCTION, "");
    mu_assert(biseqcstr(node->function.name, "foo"), "");
    mu_assert(biseqcstr(node->function.return_type, "bar"), "");
    mu_assert(node->function.arg_count == 2, "");
    mu_assert(node->function.args[0] == farg1, "");
    mu_assert(node->function.args[1] == farg2, "");
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