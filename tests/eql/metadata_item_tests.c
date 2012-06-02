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

int test_eql_ast_metadata_item_create() {
    eql_ast_node *node;
    eql_ast_metadata_item_create(&foo, &bar, &node);
    mu_assert(node->type == EQL_AST_TYPE_METADATA_ITEM, "");
    mu_assert(biseqcstr(node->metadata_item.key, "foo"), "");
    mu_assert(biseqcstr(node->metadata_item.value, "bar"), "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_metadata_item_create);
    return 0;
}

RUN_TESTS()