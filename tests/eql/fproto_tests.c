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
struct tagbstring baz = bsStatic("baz");
struct tagbstring bat = bsStatic("bat");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_fproto_create() {
    eql_ast_node *args[2];
    eql_ast_node *node, *farg1, *var_decl1, *farg2, *var_decl2;
    eql_ast_var_decl_create(&foo, &bar, &var_decl1);
    eql_ast_farg_create(var_decl1, &farg1);
    eql_ast_var_decl_create(&foo, &bar, &var_decl2);
    eql_ast_farg_create(var_decl2, &farg2);
    args[0] = farg1;
    args[1] = farg2;
    eql_ast_fproto_create(&baz, &bat, args, 2, &node);

    mu_assert(node->type == EQL_AST_TYPE_FPROTO, "");
    mu_assert(biseqcstr(node->fproto.name, "baz"), "");
    mu_assert(biseqcstr(node->fproto.return_type, "bat"), "");
    mu_assert(node->fproto.arg_count == 2, "");
    mu_assert(node->fproto.args[0] == farg1, "");
    mu_assert(node->fproto.args[1] == farg2, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_fproto_create);
    return 0;
}

RUN_TESTS()