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

int test_eql_ast_method_create() {
    eql_ast_node *node, *function;
    eql_ast_function_create(&foo, &bar, NULL, 0, NULL, &function);
    eql_ast_method_create(EQL_ACCESS_PUBLIC, function, &node);
    mu_assert(node->type == EQL_AST_TYPE_METHOD, "");
    mu_assert(node->method.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(node->method.function == function, "");
    mu_assert(node->method.function->parent == node, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_method() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("class Foo { private Int foo() { 1 + 3; } }");
    eql_parse(NULL, text, &module);

    eql_ast_node *class = module->module.classes[0];
    mu_assert(class->class.method_count == 1, "");

    eql_ast_node *node = class->class.methods[0];
    mu_assert(node->type == EQL_AST_TYPE_METHOD, "");
    mu_assert(node->method.access == EQL_ACCESS_PRIVATE, "");
    mu_assert(biseqcstr(node->method.function->function.name, "foo"), "");
    mu_assert(biseqcstr(node->method.function->function.return_type, "Int"), "");

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
    mu_run_test(test_eql_ast_method_create);
    mu_run_test(test_eql_parse_method);
    return 0;
}

RUN_TESTS()