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

int test_eql_ast_class_create() {
    eql_ast_node *methods[1];
    eql_ast_node *properties[2];
    eql_ast_node *node, *method1, *property1, *property2;
    eql_ast_method_create(EQL_ACCESS_PUBLIC, NULL, &method1);
    methods[0] = method1;
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property1);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, NULL, &property2);
    properties[0] = property1;
    properties[1] = property2;
    eql_ast_class_create(&Foo, methods, 1, properties, 2, &node);
    
    mu_assert(node->type == EQL_AST_TYPE_CLASS, "");
    mu_assert(biseqcstr(node->class.name, "Foo"), "");
    mu_assert(node->class.method_count == 1, "");
    mu_assert(node->class.methods[0] == method1, "");
    mu_assert(node->class.property_count == 2, "");
    mu_assert(node->class.properties[0] == property1, "");
    mu_assert(node->class.properties[1] == property2, "");
    eql_ast_node_free(node);
    return 0;
}




//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_class_create);
    return 0;
}

RUN_TESTS()