#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>

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
    mu_assert(node->metadata.items[1] == item2, "");
    eql_ast_node_free(node);
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_metadata_create);
    return 0;
}

RUN_TESTS()