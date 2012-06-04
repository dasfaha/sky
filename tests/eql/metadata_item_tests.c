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

int test_eql_ast_metadata_item_create() {
    eql_ast_node *node;
    eql_ast_metadata_item_create(&foo, &bar, &node);
    mu_assert(node->type == EQL_AST_TYPE_METADATA_ITEM, "");
    mu_assert(biseqcstr(node->metadata_item.key, "foo"), "");
    mu_assert(biseqcstr(node->metadata_item.value, "bar"), "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_metadata_item() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("[Baz(firstName=\"bar\", lastName=\"bat\")] class Foo { }");
    eql_parse(NULL, text, &module);
    eql_ast_node *class = module->module.classes[0];
    eql_ast_node *metadata = class->class.metadatas[0];
    mu_assert(metadata->metadata.item_count == 2, "");
    mu_assert(biseqcstr(metadata->metadata.items[0]->metadata_item.key, "firstName"), "");
    mu_assert(biseqcstr(metadata->metadata.items[0]->metadata_item.value, "bar"), "");
    mu_assert(biseqcstr(metadata->metadata.items[1]->metadata_item.key, "lastName"), "");
    mu_assert(biseqcstr(metadata->metadata.items[1]->metadata_item.value, "bat"), "");
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
    mu_run_test(test_eql_ast_metadata_item_create);
    mu_run_test(test_eql_parse_metadata_item);
    return 0;
}

RUN_TESTS()