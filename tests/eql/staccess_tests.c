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
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_staccess_create() {
    eql_ast_node *node, *var_ref;
    eql_ast_var_ref_create(&foo, &var_ref);
    eql_ast_staccess_create(var_ref, &bar, &node);
    mu_assert(node->type == EQL_AST_TYPE_STACCESS, "");
    mu_assert(node->staccess.var_ref == var_ref, "");
    mu_assert(node->staccess.var_ref->parent == node, "");
    mu_assert(biseqcstr(node->staccess.member_name, "bar"), "");
    mu_assert_eql_node_dump(node,
        "<staccess member-name='bar'>\n"
        "  <var-ref name='foo'>\n"
    );
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_staccess() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("foo.bar;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_STACCESS, "");
    mu_assert(biseqcstr(node->staccess.var_ref->var_ref.name, "foo"), "");
    mu_assert(biseqcstr(node->staccess.member_name, "bar"), "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_staccess_dump() {
    mu_assert_eql_ast_dump(
        "class Foo { public Int bar; public Int baz; } Foo x; x.baz = 20; return x.baz;",
        "<module name=''>\n"
        "  <class name='Foo'>\n"
        "    <property>\n"
        "      <var-decl type='Int' name='bar'>\n"
        "    <property>\n"
        "      <var-decl type='Int' name='baz'>\n"
        "<function name='main' return-type='Foo'>\n"
        "  <block name=''>\n"
        "    <var-decl type='Foo' name='x'>\n"
        "    <var-assign>\n"
        "      <staccess member-name='baz'>\n"
        "        <var-ref name='x'>\n"
        "      <int-literal value='20'>\n"
        "    <freturn>\n"
        "      <staccess member-name='baz'>\n"
        "        <var-ref name='x'>\n"
    );
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_staccess() {
    mu_assert_eql_compile("class Foo { public Int bar; public Int baz; } Foo x; x.baz = 20; return x.baz;", "tests/fixtures/eql/ir/staccess.ll");
    return 0;
}


//--------------------------------------
// Execute
//--------------------------------------

int test_eql_execute_staccess() {
    mu_assert_eql_execute_int("class Foo { public Int bar; public Int baz; } Foo x; x.baz = 20; return x.baz;", 20);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_staccess_create);
    mu_run_test(test_eql_parse_staccess);
    mu_run_test(test_eql_parse_staccess_dump);
    mu_run_test(test_eql_compile_staccess);
    //mu_run_test(test_eql_execute_staccess);
    return 0;
}

RUN_TESTS()