#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <eql/ast.h>
#include <mem.h>

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
// Int Literal
//--------------------------------------

int test_eql_ast_int_literal_create() {
    eql_ast_node *node;
    eql_ast_int_literal_create(20, &node);
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
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
    mu_assert(node->type == EQL_AST_TYPE_FLOAT_LITERAL, "");
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
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs == lhs, "");
    mu_assert(node->binary_expr.rhs == rhs, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Variable Reference
//--------------------------------------

int test_eql_ast_var_ref_create() {
    eql_ast_node *node;
    eql_ast_var_ref_create(&foo, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_REF, "");
    mu_assert(biseqcstr(node->var_ref.name, "foo"), "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Variable Declaration
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


//--------------------------------------
// Function Argument Declaration
//--------------------------------------

int test_eql_ast_farg_create() {
    eql_ast_node *node, *var_decl;
    eql_ast_var_decl_create(&foo, &bar, &var_decl);
    eql_ast_farg_create(var_decl, &node);
    mu_assert(node->type == EQL_AST_TYPE_FARG, "");
    mu_assert(node->farg.var_decl == var_decl, "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Function Prototype
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
    mu_run_test(test_eql_ast_int_literal_create);
    mu_run_test(test_eql_ast_float_literal_create);
    mu_run_test(test_eql_ast_binary_expr_create);
    mu_run_test(test_eql_ast_var_ref_create);
    return 0;
}

RUN_TESTS()