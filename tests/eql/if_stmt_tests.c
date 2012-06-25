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


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// AST
//--------------------------------------

int test_eql_ast_if_stmt_create() {
    eql_ast_node *node, *condition, *block;
    eql_ast_var_ref_create(&foo, &condition);
    eql_ast_block_create(&bar, NULL, 0, &block);
    eql_ast_if_stmt_create(condition, block, &node);

    mu_assert(node->type == EQL_AST_TYPE_IF_STMT, "");
    mu_assert(node->if_stmt.condition == condition, "");
    mu_assert(node->if_stmt.condition->parent == node, "");
    mu_assert(node->if_stmt.block == block, "");
    mu_assert(node->if_stmt.block->parent == node, "");

    mu_assert_eql_node_dump(node,
        "<if-stmt>\n"
        "  <var-ref name='foo'>\n"
        "  <block name='bar'>\n"
    );

    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_if_stmt() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("Int foo; if(foo == 2) { foo = 3; } return foo;");
    eql_parse(NULL, text, &module);
    mu_assert_eql_node_dump(module,
        "<module name=''>\n"
        "<function name='main' return-type=''>\n"
        "  <block name=''>\n"
        "    <var-decl type='Int' name='foo'>\n"
        "    <if-stmt>\n"
        "      <binary-expr operator='=='>\n"
        "        <var-ref name='foo'>\n"
        "        <int-literal value='2'>\n"
        "      <block name=''>\n"
        "        <var-assign>\n"
        "          <var-ref name='foo'>\n"
        "          <int-literal value='3'>\n"
        "    <freturn>\n"
        "      <var-ref name='foo'>\n"
    );
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
    mu_run_test(test_eql_ast_if_stmt_create);
    mu_run_test(test_eql_parse_if_stmt);
    return 0;
}

RUN_TESTS()