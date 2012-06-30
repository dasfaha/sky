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
    eql_ast_var_decl_create(&foo, &bar, NULL, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_DECL, "");
    mu_assert(biseqcstr(node->var_decl.type, "foo"), "");
    mu_assert(biseqcstr(node->var_decl.name, "bar"), "");
    mu_assert_eql_node_dump(node, "<var-decl type='foo' name='bar'>\n");
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
    mu_assert_eql_node_dump(module,
        "<module name=''>\n"
        "<function name='main' return-type=''>\n"
        "  <block name=''>\n"
        "    <var-decl type='Int' name='myVar_26'>\n"
    );
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_var_decl_with_initial_value() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("Int myVar = 100;");
    eql_parse(NULL, text, &module);
    mu_assert_eql_node_dump(module,
        "<module name=''>\n"
        "<function name='main' return-type=''>\n"
        "  <block name=''>\n"
        "    <var-decl type='Int' name='myVar'>\n"
        "      <int-literal value='100'>\n"
    );
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_var_decl() {
    mu_assert_eql_compile("Int foo; Float bar; return 200;", "tests/fixtures/eql/ir/var_decl.ll");
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
    mu_run_test(test_eql_parse_var_decl_with_initial_value);
    mu_run_test(test_eql_compile_var_decl);
    return 0;
}

RUN_TESTS()