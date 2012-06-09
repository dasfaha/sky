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


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_var_decl() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("Int myVar_26;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_VAR_DECL, "");
    mu_assert(biseqcstr(node->var_decl.type, "Int"), "");
    mu_assert(biseqcstr(node->var_decl.name, "myVar_26"), "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_var_decl_create);
    mu_run_test(test_eql_parse_var_decl);
    return 0;
}

RUN_TESTS()