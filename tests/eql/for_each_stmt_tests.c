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

struct tagbstring Foo = bsStatic("Foo");
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

int test_eql_ast_for_each_stmt_create() {
    eql_ast_node *node, *var_decl, *enumerator, *block;
    eql_ast_var_decl_create(&Foo, &foo, &var_decl);
    eql_ast_var_ref_create(&bar, &enumerator);
    eql_ast_block_create(NULL, NULL, 0, &block);
    eql_ast_for_each_stmt_create(var_decl, enumerator, block, &node);

    mu_assert(node->type == EQL_AST_TYPE_FOR_EACH_STMT, "");
    mu_assert(node->for_each_stmt.var_decl == var_decl, "");
    mu_assert(node->for_each_stmt.var_decl->parent == node, "");
    mu_assert(node->for_each_stmt.enumerator == enumerator, "");
    mu_assert(node->for_each_stmt.enumerator->parent == node, "");
    mu_assert(node->for_each_stmt.block == block, "");
    mu_assert(node->for_each_stmt.block->parent == node, "");

    mu_assert_eql_node_dump(node,
        "<for-each-stmt>\n"
        "  <var-decl type='Foo' name='foo'>\n"
        "  <var-ref name='bar'>\n"
        "  <block name=''>\n"
    );

    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_for_each_stmt() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("Int foo; for each(Int i in foo) { foo = foo + 1; } return;");
    eql_parse(NULL, text, &module);
    mu_assert_eql_node_dump(module,
        "<module name=''>\n"
        "<function name='main' return-type=''>\n"
        "  <block name=''>\n"
        "    <var-decl type='Int' name='foo'>\n"
        "    <for-each-stmt>\n"
        "      <var-decl type='Int' name='i'>\n"
        "      <var-ref name='foo'>\n"
        "      <block name=''>\n"
        "        <var-assign>\n"
        "          <var-ref name='foo'>\n"
        "          <binary-expr operator='+'>\n"
        "            <var-ref name='foo'>\n"
        "            <int-literal value='1'>\n"
        "    <freturn>\n"
    );
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}



//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_for_each_stmt() {
    mu_assert_eql_compile("", "tests/fixtures/eql/ir/for_each_stmt.ll");
    return 0;
}




//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_for_each_stmt_create);
    mu_run_test(test_eql_parse_for_each_stmt);
    //mu_run_test(test_eql_compile_for_each_stmt);
    return 0;
}

RUN_TESTS()