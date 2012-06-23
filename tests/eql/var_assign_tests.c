#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>
#include <eql/compiler.h>

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

int test_eql_ast_var_assign_create() {
    eql_ast_node *node, *var_ref, *int_literal;
    eql_ast_var_ref_create(&foo, &var_ref);
    eql_ast_int_literal_create(10, &int_literal);
    eql_ast_var_assign_create(var_ref, int_literal, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_ASSIGN, "");
    mu_assert(node->var_assign.var_ref == var_ref, "");
    mu_assert(node->var_assign.var_ref->parent == node, "");
    mu_assert(node->var_assign.expr == int_literal, "");
    mu_assert(node->var_assign.expr->parent == node, "");
    mu_assert_eql_node_dump(node,
        "<var-assign>\n"
        "  <var-ref name='foo'>\n"
        "  <int-literal value='10'>\n"
    );
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_var_assign() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("foo = 20;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_VAR_ASSIGN, "");
    mu_assert(biseqcstr(node->var_assign.var_ref->var_ref.name, "foo"), "");
    mu_assert(node->var_assign.expr->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->var_assign.expr->int_literal.value == 20, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_var_assign() {
    mu_assert_eql_compile("Int foo; foo = 20; return foo;", "tests/fixtures/eql/ir/var_assign.ll");
    return 0;
}

//--------------------------------------
// Execute
//--------------------------------------

int test_eql_execute_var_assign() {
    mu_assert_eql_execute_int("Int foo; foo = 20; foo = foo + 5; return foo;", 25);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_var_assign_create);
    mu_run_test(test_eql_parse_var_assign);
    mu_run_test(test_eql_compile_var_assign);
    mu_run_test(test_eql_execute_var_assign);
    return 0;
}

RUN_TESTS()