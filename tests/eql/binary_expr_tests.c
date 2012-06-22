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
// AST
//--------------------------------------

int test_eql_ast_binary_expr_create() {
    eql_ast_node *node, *lhs, *rhs;
    eql_ast_int_literal_create(10, &lhs);
    eql_ast_float_literal_create(10, &rhs);
    eql_ast_binary_expr_create(EQL_BINOP_MUL, lhs, rhs, &node);
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs == lhs, "");
    mu_assert(node->binary_expr.lhs->parent == node, "");
    mu_assert(node->binary_expr.rhs == rhs, "");
    mu_assert(node->binary_expr.rhs->parent == node, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Parser
//--------------------------------------

int test_eql_parse_binary_expr_plus() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("10+3;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_PLUS, "");
    mu_assert(node->binary_expr.lhs->int_literal.value == 10, "");
    mu_assert(node->binary_expr.rhs->int_literal.value == 3, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_binary_expr_minus() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("19 - 2;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MINUS, "");
    mu_assert(node->binary_expr.lhs->int_literal.value == 19, "");
    mu_assert(node->binary_expr.rhs->int_literal.value == 2, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_binary_expr_mul() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("1 * 2;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs->int_literal.value == 1, "");
    mu_assert(node->binary_expr.rhs->int_literal.value == 2, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_binary_expr_div() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("10.2 / 4;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_DIV, "");
    mu_assert(node->binary_expr.lhs->float_literal.value == 10.2, "");
    mu_assert(node->binary_expr.rhs->int_literal.value == 4, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_binary_expr_complex() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("1 + 2 * 3;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_PLUS, "");
    mu_assert(node->binary_expr.lhs->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->binary_expr.lhs->int_literal.value == 1, "");
    mu_assert(node->binary_expr.rhs->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.rhs->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.rhs->binary_expr.lhs->int_literal.value == 2, "");
    mu_assert(node->binary_expr.rhs->binary_expr.rhs->int_literal.value == 3, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

int test_eql_parse_binary_expr_parens() {
    eql_ast_node *module = NULL;
    bstring text = bfromcstr("(1 + 2) * 3;");
    eql_parse(NULL, text, &module);
    eql_ast_node *node = module->module.main_function->function.body->block.exprs[0];
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.lhs->binary_expr.operator == EQL_BINOP_PLUS, "");
    mu_assert(node->binary_expr.lhs->binary_expr.lhs->int_literal.value == 1, "");
    mu_assert(node->binary_expr.lhs->binary_expr.rhs->int_literal.value == 2, "");
    mu_assert(node->binary_expr.rhs->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->binary_expr.rhs->int_literal.value == 3, "");
    eql_ast_node_free(module);
    bdestroy(text);
    return 0;
}

//--------------------------------------
// Compile
//--------------------------------------

int test_eql_compile_binary_expr_int() {
    mu_assert_eql_compile("return 1 + 2;", "tests/fixtures/eql/ir/binary_expr_int.ll");
    return 0;
}

int test_eql_compile_binary_expr_float() {
    mu_assert_eql_compile("return 1.5 + 2.2;", "tests/fixtures/eql/ir/binary_expr_float.ll");
    return 0;
}

int test_eql_compile_binary_expr_int_cast() {
    mu_assert_eql_compile("return 1 + 2.5;", "tests/fixtures/eql/ir/binary_expr_int_cast.ll");
    return 0;
}

int test_eql_compile_binary_expr_float_cast() {
    mu_assert_eql_compile("return 2.5 + 1;", "tests/fixtures/eql/ir/binary_expr_float_cast.ll");
    return 0;
}

int test_eql_compile_binary_expr_sub() {
    mu_assert_eql_compile("return 10 - 5;", "tests/fixtures/eql/ir/binary_expr_sub.ll");
    return 0;
}

int test_eql_compile_binary_expr_mul() {
    mu_assert_eql_compile("return 10 * 5;", "tests/fixtures/eql/ir/binary_expr_mul.ll");
    return 0;
}

int test_eql_compile_binary_expr_div() {
    mu_assert_eql_compile("return 100 / 5;", "tests/fixtures/eql/ir/binary_expr_div.ll");
    return 0;
}


//--------------------------------------
// Type
//--------------------------------------

int test_eql_binary_expr_get_type() {
    bstring type;
    eql_ast_node *node, *lhs, *rhs;
    eql_ast_int_literal_create(10, &lhs);
    eql_ast_float_literal_create(10, &rhs);
    eql_ast_binary_expr_create(EQL_BINOP_MUL, lhs, rhs, &node);
    eql_ast_node_get_type(node, &type);
    mu_assert(biseqcstr(type, "Int"), "");
    eql_ast_node_free(node);
    bdestroy(type);
    return 0;
}

//--------------------------------------
// Execute  
//--------------------------------------

int test_eql_execute_binary_expr() {
    mu_assert_eql_execute_int("return 100 + 5 * (3 + 1);", 120);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_binary_expr_create);
    mu_run_test(test_eql_parse_binary_expr_plus);
    mu_run_test(test_eql_parse_binary_expr_minus);
    mu_run_test(test_eql_parse_binary_expr_mul);
    mu_run_test(test_eql_parse_binary_expr_div);
    mu_run_test(test_eql_parse_binary_expr_complex);
    mu_run_test(test_eql_parse_binary_expr_parens);
    mu_run_test(test_eql_compile_binary_expr_int);
    mu_run_test(test_eql_compile_binary_expr_float);
    mu_run_test(test_eql_compile_binary_expr_int_cast);
    mu_run_test(test_eql_compile_binary_expr_float_cast);
    mu_run_test(test_eql_compile_binary_expr_sub);
    mu_run_test(test_eql_compile_binary_expr_mul);
    mu_run_test(test_eql_compile_binary_expr_div);
    mu_run_test(test_eql_binary_expr_get_type);
    mu_run_test(test_eql_execute_binary_expr);
    return 0;
}

RUN_TESTS()