#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>

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

int test_eql_ast_fcall_create() {
    eql_ast_node *args[2];
    eql_ast_node *node, *expr1, *expr2;
    eql_ast_int_literal_create(10, &expr1);
    eql_ast_int_literal_create(20, &expr2);
    args[0] = expr1;
    args[1] = expr2;
    eql_ast_fcall_create(&foo, args, 2, &node);

    mu_assert(node->type == EQL_AST_TYPE_FCALL, "");
    mu_assert(biseqcstr(node->fcall.name, "foo"), "");
    mu_assert(node->fcall.arg_count == 2, "");
    mu_assert(node->fcall.args[0] == expr1, "");
    mu_assert(node->fcall.args[0]->parent == node, "");
    mu_assert(node->fcall.args[1] == expr2, "");
    mu_assert(node->fcall.args[1]->parent == node, "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_fcall() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("foo(20, bar+2);");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_FCALL, "");
    mu_assert(biseqcstr(node->fcall.name, "foo"), "");
    mu_assert(node->fcall.arg_count == 2, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_fcall() {
    mu_assert_eql_compile("class Foo { public Int foo() {return 20;} public Int bar() {return foo();} } Foo x; return x;", "tests/fixtures/eql/ir/fcall.ll");
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_fcall_create);
    mu_run_test(test_eql_parse_fcall);
    mu_run_test(test_eql_compile_fcall);
    return 0;
}

RUN_TESTS()