#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <eql/ast.h>
#include <mem.h>

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
// Int Literal
//--------------------------------------

int test_eql_ast_int_literal_create() {
    eql_ast_node *node;
    eql_ast_int_literal_create(20, &node);
    mu_assert(node->type == EQL_AST_TYPE_INT_LITERAL, "");
    mu_assert(node->int_literal.value == 20, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Float Literal
//--------------------------------------

int test_eql_ast_float_literal_create() {
    eql_ast_node *node;
    eql_ast_float_literal_create(100.293, &node);
    mu_assert(node->type == EQL_AST_TYPE_FLOAT_LITERAL, "");
    mu_assert(node->float_literal.value == 100.293, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Binary Expression
//--------------------------------------

int test_eql_ast_binary_expr_create() {
    eql_ast_node *node, *lhs, *rhs;
    eql_ast_int_literal_create(10, &lhs);
    eql_ast_float_literal_create(10, &rhs);
    eql_ast_binary_expr_create(EQL_BINOP_MUL, lhs, rhs, &node);
    mu_assert(node->type == EQL_AST_TYPE_BINARY_EXPR, "");
    mu_assert(node->binary_expr.operator == EQL_BINOP_MUL, "");
    mu_assert(node->binary_expr.lhs == lhs, "");
    mu_assert(node->binary_expr.rhs == rhs, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Variable Reference
//--------------------------------------

int test_eql_ast_var_ref_create() {
    eql_ast_node *node;
    eql_ast_var_ref_create(&foo, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_REF, "");
    mu_assert(biseqcstr(node->var_ref.name, "foo"), "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Variable Declaration
//--------------------------------------

int test_eql_ast_var_decl_create() {
    eql_ast_node *node;
    eql_ast_var_decl_create(&foo, &bar, &node);
    mu_assert(node->type == EQL_AST_TYPE_VAR_DECL, "");
    mu_assert(biseqcstr(node->var_decl.type, "foo"), "");
    mu_assert(biseqcstr(node->var_decl.name, "bar"), "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Function Argument Declaration
//--------------------------------------

int test_eql_ast_farg_create() {
    eql_ast_node *node, *var_decl;
    eql_ast_var_decl_create(&foo, &bar, &var_decl);
    eql_ast_farg_create(var_decl, &node);
    mu_assert(node->type == EQL_AST_TYPE_FARG, "");
    mu_assert(node->farg.var_decl == var_decl, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Function Prototype
//--------------------------------------

int test_eql_ast_fproto_create() {
    eql_ast_node *args[2];
    eql_ast_node *node, *farg1, *var_decl1, *farg2, *var_decl2;
    eql_ast_var_decl_create(&foo, &bar, &var_decl1);
    eql_ast_farg_create(var_decl1, &farg1);
    eql_ast_var_decl_create(&foo, &bar, &var_decl2);
    eql_ast_farg_create(var_decl2, &farg2);
    args[0] = farg1;
    args[1] = farg2;
    eql_ast_fproto_create(&baz, &bat, args, 2, &node);

    mu_assert(node->type == EQL_AST_TYPE_FPROTO, "");
    mu_assert(biseqcstr(node->fproto.name, "baz"), "");
    mu_assert(biseqcstr(node->fproto.return_type, "bat"), "");
    mu_assert(node->fproto.arg_count == 2, "");
    mu_assert(node->fproto.args[0] == farg1, "");
    mu_assert(node->fproto.args[1] == farg2, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Function
//--------------------------------------

int test_eql_ast_function_create() {
    eql_ast_node *node, *prototype, *body;
    eql_ast_fproto_create(&foo, &bar, NULL, 0, &prototype);
    eql_ast_int_literal_create(10, &body);
    eql_ast_function_create(prototype, body, &node);
    mu_assert(node->type == EQL_AST_TYPE_FUNCTION, "");
    mu_assert(node->function.prototype == prototype, "");
    mu_assert(node->function.body == body, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Function Call
//--------------------------------------

int test_eql_ast_fcall_create() {
    eql_ast_node *args[2];
    eql_ast_node *node, *expr1, *expr2;
    eql_ast_int_literal_create(10, &expr1);
    eql_ast_int_literal_create(20, &expr2);
    args[0] = expr1;
    args[1] = expr2;
    eql_ast_fcall_create(&foo, args, 2, &node);

    mu_assert(node->type == EQL_AST_TYPE_FCALL, "");
    mu_assert(biseqcstr(node->fcall.name, "foo"), "");
    mu_assert(node->fcall.arg_count == 2, "");
    mu_assert(node->fcall.args[0] == expr1, "");
    mu_assert(node->fcall.args[1] == expr2, "");
    eql_ast_node_free(node);
    return 0;
}

//--------------------------------------
// Block
//--------------------------------------

int test_eql_ast_block_create() {
    eql_ast_node *exprs[2];
    eql_ast_node *node, *expr1, *expr2;
    eql_ast_int_literal_create(10, &expr1);
    eql_ast_int_literal_create(20, &expr2);
    exprs[0] = expr1;
    exprs[1] = expr2;
    eql_ast_block_create(exprs, 2, &node);
    mu_assert(node->type == EQL_AST_TYPE_BLOCK, "");
    mu_assert(node->block.expr_count == 2, "");
    mu_assert(node->block.exprs[0] == expr1, "");
    mu_assert(node->block.exprs[1] == expr2, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Method
//--------------------------------------

int test_eql_ast_method_create() {
    eql_ast_node *node, *function;
    eql_ast_function_create(NULL, NULL, &function);
    eql_ast_method_create(EQL_ACCESS_PUBLIC, function, &node);
    mu_assert(node->type == EQL_AST_TYPE_METHOD, "");
    mu_assert(node->method.access == EQL_ACCESS_PUBLIC, "");
    mu_assert(node->method.function == function, "");
    eql_ast_node_free(node);
    return 0;
}


//--------------------------------------
// Property
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
// Class
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


//--------------------------------------
// Module
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
    mu_assert(node->module.class_count == 1, "");
    mu_assert(node->module.classes[0] == class1, "");
    mu_assert(node->module.classes[1] == class2, "");
    mu_assert(node->module.block == block, "");
    eql_ast_node_free(node);
    return 0;
}


//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_eql_ast_int_literal_create);
    mu_run_test(test_eql_ast_float_literal_create);
    mu_run_test(test_eql_ast_binary_expr_create);
    mu_run_test(test_eql_ast_var_ref_create);
    mu_run_test(test_eql_ast_var_decl_create);
    mu_run_test(test_eql_ast_farg_create);
    mu_run_test(test_eql_ast_fproto_create);
    mu_run_test(test_eql_ast_function_create);
    mu_run_test(test_eql_ast_fcall_create);
    mu_run_test(test_eql_ast_block_create);
    mu_run_test(test_eql_ast_method_create);
    mu_run_test(test_eql_ast_property_create);
    mu_run_test(test_eql_ast_class_create);
    return 0;
}

RUN_TESTS()