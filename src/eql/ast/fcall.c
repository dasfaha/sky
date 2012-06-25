#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring BSTR_THIS = bsStatic("this");


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a function call.
//
// name      - The name of the function that is being called.
// args      - An array of argument values passed to the function.
// arg_count - The number of arguments passed to the function.
// ret       - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_fcall_create(bstring name, struct eql_ast_node **args,
                         unsigned int arg_count, struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FCALL;
    node->parent = NULL;
    node->fcall.name = bstrcpy(name); check_mem(node->fcall.name);

    // Copy arguments.
    if(arg_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * arg_count;
        node->fcall.args = malloc(sz);
        check_mem(node->fcall.args);
        
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->fcall.args[i] = args[i];
            args[i]->parent = node;
        }
    }
    else {
        node->fcall.args = NULL;
    }
    node->fcall.arg_count = arg_count;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void eql_ast_fcall_free(struct eql_ast_node *node)
{
    if(node->fcall.name) bdestroy(node->fcall.name);
    node->fcall.name = NULL;
    
    if(node->fcall.arg_count > 0) {
        unsigned int i;
        for(i=0; i<node->fcall.arg_count; i++) {
            eql_ast_node_free(node->fcall.args[i]);
            node->fcall.args[i] = NULL;
        }
        free(node->fcall.args);
        node->fcall.arg_count = 0;
    }
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the function call AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_fcall_codegen(eql_ast_node *node, eql_module *module,
                            LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_FCALL, "Node type expected to be 'function call'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Retrieve the object this function is being called by.
    eql_ast_node *var_decl;
    LLVMValueRef var_decl_value;
    rc = eql_module_get_variable(module, &BSTR_THIS, &var_decl, &var_decl_value);
    check(rc == 0, "Unable to retrieve variable declaration: this");
    check(var_decl != NULL, "No variable declaration found: this");
    check(var_decl_value != NULL, "No LLVM value for variable declaration: this");

    // Determine function name.
    bstring function_name = bformat("%s___%s", bdata(var_decl->var_decl.type), bdata(node->fcall.name));
    check_mem(function_name);
    
    // Create load instruction for target object.
    LLVMValueRef this_value = LLVMBuildLoad(builder, var_decl_value, "");
    check(this_value != NULL, "Unable to create load instruction");

    // Offset for methods.
    int offset = 1;

    // Retrieve function.
    LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    check(func != NULL, "Unable to find function: %s", bdata(function_name));
    check(LLVMCountParams(func) == node->fcall.arg_count+offset, "Argument mismatch (got %d, expected %d)", node->fcall.arg_count, LLVMCountParams(func));

    // Allocate arguments.
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * (node->fcall.arg_count+offset));

    // Pass 'this' as first argument.
    args[0] = this_value;

    // Evaluate arguments.
    unsigned int i;
    unsigned int arg_count = node->fcall.arg_count;
    for(i=0; i<arg_count; i++) {
        rc = eql_ast_node_codegen(node->fcall.args[i], module, &args[i+offset]);
        check(rc == 0, "Unable to codegen argument: %d", i);
    }

    // Create call instruction.
    *value = LLVMBuildCall(builder, func, args, arg_count+offset, "");

    return 0;

error:
    *value = NULL;
    if(args) free(args);
    args = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node   - The AST node to determine the type for.
// module - The compilation unit this node is a part of.
// type   - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_fcall_get_type(eql_ast_node *node, eql_module *module,
                           bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_FCALL, "Node type must be 'function call'");

    // Find class
    eql_ast_node *parent = node->parent;
    while(parent != NULL) {
        if(parent->type == EQL_AST_TYPE_CLASS) {
            *type = parent->class.name;
            break;
        }

        parent = parent->parent;
    }

    sentinel("Unable to find function call type: %s", bdata(node->fcall.name));

error:
    *type = NULL;
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
int eql_ast_fcall_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<fcall name='%s'>\n", bdatae(node->fcall.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->fcall.arg_count; i++) {
        rc = eql_ast_node_dump(node->fcall.args[i], ret);
        check(rc == 0, "Unable to dump fcall argument");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
