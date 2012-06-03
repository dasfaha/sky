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

int test_eql_ast_property_create() {
    eql_ast_node *node, *var_decl;
    eql_ast_var_decl_create(&foo, &bar, &var_decl);
    eql_ast_property_create(EQL_ACCESS_PUBLIC, var_decl, &node);
    mu_assert(node->type == EQL_AST_TYPE_PROPERTY, "");
    mu_assert(node->property.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(node->property.var_decl == var_decl, "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_property() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("class Foo { public Float bar; }");
    eql_parse(NULL, text, &module);

    eql_ast_node *class = module->module.classes[0];
    mu_assert(class->class.property_count == 1, "");

    eql_ast_node *node = class->class.properties[0];
    mu_assert(node->type == EQL_AST_TYPE_PROPERTY, "");
    mu_assert(node->property.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(biseqcstr(node->property.var_decl->var_decl.name, "bar"), "");
    mu_assert(biseqcstr(node->property.var_decl->var_decl.type, "Float"), "");

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
    mu_run_test(test_eql_ast_property_create);
    mu_run_test(test_eql_parse_property);
    return 0;
}

RUN_TESTS()