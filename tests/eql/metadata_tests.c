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

struct tagbstring Foo = bsStatic("Foo");
struct tagbstring Bar = bsStatic("Bar");

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
// AST
//--------------------------------------

int test_eql_ast_metadata_create() {
    eql_ast_node *items[2];
    eql_ast_node *node, *item1, *item2;
    eql_ast_metadata_item_create(&foo, &bar, &item1);
    eql_ast_metadata_item_create(&foo, &baz, &item2);
    items[0] = item1;
    items[1] = item2;
    eql_ast_metadata_create(&Foo, items, 2, &node);
    mu_assert(node->type == EQL_AST_TYPE_METADATA, "");
    mu_assert(biseqcstr(node->metadata.name, "Foo"), "");
    mu_assert(node->metadata.item_count == 2, "");
    mu_assert(node->metadata.items[0] == item1, "");
    mu_assert(node->metadata.items[0]->parent == node, "");
    mu_assert(node->metadata.items[1] == item2, "");
    mu_assert(node->metadata.items[1]->parent == node, "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_metadata() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("[Baz] [Bat] class Foo { }");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.classes[0];
    mu_assert(node->type == EQL_AST_TYPE_CLASS, "");
    mu_assert(biseqcstr(node->class.name, "Foo"), "");
    mu_assert(node->class.metadata_count == 2, "");
    mu_assert(biseqcstr(node->class.metadatas[0]->metadata.name, "Baz"), "");
    mu_assert(biseqcstr(node->class.metadatas[1]->metadata.name, "Bat"), "");
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
    mu_run_test(test_eql_ast_metadata_create);
    mu_run_test(test_eql_parse_metadata);
    return 0;
}

RUN_TESTS()