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
// ÅST
//--------------------------------------

int test_eql_ast_module_create() {
    eql_ast_node *classes[2];
    eql_ast_node *node, *class1, *class2, *block;
    eql_ast_class_create(&Foo, NULL, 0, NULL, 0, &class1);
    eql_ast_class_create(&Bar, NULL, 0, NULL, 0, &class2);
    classes[0] = class1;
    classes[1] = class2;
    eql_ast_block_create(NULL, 0, &block);
    eql_ast_module_create(&bar, classes, 2, block, &node);
    
    mu_assert(node->type == EQL_AST_TYPE_MODULE, "");
    mu_assert(biseqcstr(node->class.name, "bar"), "");
    mu_assert(node->module.class_count == 2, "");
    mu_assert(node->module.classes[0] == class1, "");
    mu_assert(node->module.classes[1] == class2, "");
    mu_assert(node->module.block == block, "");
    eql_ast_node_free(node);
    return 0;
}

int test_eql_ast_module_add_class() {
    eql_ast_node *node, *class1, *class2;
    eql_ast_class_create(&Foo, NULL, 0, NULL, 0, &class1);
    eql_ast_class_create(&Bar, NULL, 0, NULL, 0, &class2);
    eql_ast_module_create(&bar, NULL, 0, NULL, &node);
    eql_ast_module_add_class(node, class1);
    eql_ast_module_add_class(node, class2);
    mu_assert(node->module.class_count == 2, "");
    mu_assert(node->module.classes[0] == class1, "");
    mu_assert(node->module.classes[1] == class2, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_module_create);
    mu_run_test(test_eql_ast_module_add_class);
    return 0;
}

RUN_TESTS()