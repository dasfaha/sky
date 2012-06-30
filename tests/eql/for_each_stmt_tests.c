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
    mu_assert_eql_compile(
        "[Enumerable]\n"
        "class Foo {\n"
        "  private Int count;\n"
        "\n"
        "  public void next(Bar value) {\n"
        "    this.count = this.count + 1;\n"
        "    value.num = (this.count * 2);\n"
        "    return;\n"
        "  }\n"
        "  public Boolean eof() {\n"
        "    return this.count == 5;\n"
        "  }\n"
        "}\n"
        "class Bar { public Int num; }\n"
        "\n"
        "Int i;\n"
        "Foo foo;\n"
        "for each(Bar b in foo) {\n"
        "  i = b.num;\n"
        "}\n"
        "return i;"
        ,
        "tests/fixtures/eql/ir/for_each_stmt.ll"
    );
    return 0;
}


//--------------------------------------
// Execute
//--------------------------------------

int test_eql_execute_for_each_stmt() {
    mu_assert_eql_execute_int(
        "[Enumerable]\n"
        "class Foo {\n"
        "  public Int count;\n"
        "\n"
        "  public void next(Bar value) {\n"
        "    this.count = this.count + 1;\n"
        "    value.num = (this.count * 2);\n"
        "    return;\n"
        "  }\n"
        "  public Boolean eof() {\n"
        "    return this.count == 5;\n"
        "  }\n"
        "}\n"
        "class Bar { public Int num; }\n"
        "\n"
        "Int i;\n"
        "Foo foo;\n"
        "foo.count = 0;\n"
        "for each(Bar b in foo) {\n"
        "  i = b.num;\n"
        "}\n"
        "return i;"
        ,
        10
    );
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
    mu_run_test(test_eql_compile_for_each_stmt);
    mu_run_test(test_eql_execute_for_each_stmt);
    return 0;
}

RUN_TESTS()