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
    eql_ast_node *conditions[2];
    eql_ast_node *blocks[2];
    eql_ast_node *node, *condition1, *block1, *condition2, *block2, *else_block;
    eql_ast_var_ref_create(&foo, &condition1);
    eql_ast_block_create(&bar, NULL, 0, &block1);
    eql_ast_var_ref_create(&foo, &condition2);
    eql_ast_block_create(&bar, NULL, 0, &block2);
    conditions[0] = condition1;
    conditions[1] = condition2;
    blocks[0] = block1;
    blocks[1] = block2;
    eql_ast_block_create(&bar, NULL, 0, &else_block);
    eql_ast_if_stmt_create(&node);
    eql_ast_if_stmt_add_blocks(node, conditions, blocks, 2);
    eql_ast_if_stmt_set_else_block(node, else_block);

    mu_assert(node->type == EQL_AST_TYPE_IF_STMT, "");
    mu_assert(node->if_stmt.conditions[0] == condition1, "");
    mu_assert(node->if_stmt.conditions[0]->parent == node, "");
    mu_assert(node->if_stmt.blocks[0] == block1, "");
    mu_assert(node->if_stmt.blocks[0]->parent == node, "");
    mu_assert(node->if_stmt.conditions[1] == condition2, "");
    mu_assert(node->if_stmt.conditions[1]->parent == node, "");
    mu_assert(node->if_stmt.blocks[1] == block2, "");
    mu_assert(node->if_stmt.blocks[1]->parent == node, "");
    mu_assert(node->if_stmt.else_block == else_block, "");
    mu_assert(node->if_stmt.else_block->parent == node, "");

    mu_assert_eql_node_dump(node,
        "<if-stmt>\n"
        "  <var-ref name='foo'>\n"
        "  <block name='bar'>\n"
        "  <var-ref name='foo'>\n"
        "  <block name='bar'>\n"
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
        "  <block name='entry'>\n"
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

int test_eql_parse_if_else_if_stmt() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("Int foo; if(foo == 2) { foo = 3; } else if(foo == 10) { foo = 20; } return foo;");
    eql_parse(NULL, text, &module);
    mu_assert_eql_node_dump(module,
        "<module name=''>\n"
        "<function name='main' return-type=''>\n"
        "  <block name='entry'>\n"
        "    <var-decl type='Int' name='foo'>\n"
        "    <if-stmt>\n"
        "      <binary-expr operator='=='>\n"
        "        <var-ref name='foo'>\n"
        "        <int-literal value='2'>\n"
        "      <block name=''>\n"
        "        <var-assign>\n"
        "          <var-ref name='foo'>\n"
        "          <int-literal value='3'>\n"
        "      <binary-expr operator='=='>\n"
        "        <var-ref name='foo'>\n"
        "        <int-literal value='10'>\n"
        "      <block name=''>\n"
        "        <var-assign>\n"
        "          <var-ref name='foo'>\n"
        "          <int-literal value='20'>\n"
        "    <freturn>\n"
        "      <var-ref name='foo'>\n"
    );
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}


//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_if_stmt() {
    mu_assert_eql_compile("Int foo; if(foo == 1) { foo = 2; } return foo;", "tests/fixtures/eql/ir/if_stmt.ll");
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
    mu_run_test(test_eql_parse_if_else_if_stmt);
    //mu_run_test(test_eql_compile_if_stmt);
    return 0;
}

RUN_TESTS()