#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a method.
//
// function - The function.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_create(eql_ast_access_e access,
                          eql_ast_node *function,
                          eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METHOD;
    node->parent = NULL;
    node->method.access = access;
    node->method.function = function;
    if(function != NULL) {
        function->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a method AST node from memory.
//
// node - The AST node to free.
void eql_ast_method_free(struct eql_ast_node *node)
{
    if(node->method.function) eql_ast_node_free(node->method.function);
    node->method.function = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the method AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_codegen(eql_ast_node *node, eql_module *module,
                           LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(node->method.function != NULL, "Method function required");
    check(module != NULL, "Module required");

    // Delegate LLVM generation to the function.
    rc = eql_ast_function_codegen(node->method.function, module, value);
    check(rc == 0, "Unable to codegen method");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Misc
//--------------------------------------

// Generates the 'this' argument for the function the method is attached to.
// This is called after the method is attached to a class.
//
// node - The method AST node.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_method_generate_this_farg(eql_ast_node *node)
{
    int rc;
    eql_ast_node *farg, *var_decl;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_METHOD, "Node type must be 'method'");
    check(node->parent != NULL, "Method parent required");
    check(node->parent->type == EQL_AST_TYPE_CLASS, "Node parent type must be 'class'");

    eql_ast_node *function = node->method.function;

    // Only generate 'this' arg if the function exists.
    if(function != NULL) {
        // Retrieve class name for type.
        bstring type = node->parent->class.name;
        check(type != NULL, "Method's class name is required");
    
        // Create 'this' variable declaration.
        struct tagbstring THIS = bsStatic("this");
        rc = eql_ast_var_decl_create(type, &THIS, NULL, &var_decl);
        check(rc == 0, "Unable to create 'this' variable declaration");

        // Link to function argument.
        rc = eql_ast_farg_create(var_decl, &farg);
        check(rc == 0, "Unable to create 'this' function argument");
    
        // Prepend argument to function.
        function->function.arg_count++;
        function->function.args = realloc(function->function.args, sizeof(eql_ast_node*) * function->function.arg_count);
        check_mem(function->function.args);
        memmove(function->function.args+1, function->function.args, sizeof(eql_ast_node*) * (function->function.arg_count-1));
        function->function.args[0] = farg;
    }

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Debugging
//--------------------------------------

// Append the contents of the AST node to the string.
// 
// node - The node to dump.
// ret  - A pointer to the bstring to concatenate to.
//
// Return 0 if successful, otherwise returns -1.s
int eql_ast_method_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<method>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->method.function != NULL) {
        rc = eql_ast_node_dump(node->method.function, ret);
        check(rc == 0, "Unable to dump method function");
    }

    return 0;

error:
    return -1;
}
