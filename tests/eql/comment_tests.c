#include <stdio.h>
#include <stdlib.h>

#include <eql/ast.h>
#include <eql/parser.h>

#include "../minunit.h"


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_multiline_comment() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("/* this is a\nmultiline comment! */ class Foo { }");
    int rc = eql_parse(NULL, text, &module);
    mu_assert(rc == 0, "");
    mu_assert(module != NULL, "");
    mu_assert(module->module.class_count == 1, "");
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
    mu_run_test(test_eql_parse_multiline_comment);
    return 0;
}

RUN_TESTS()