#include <stdlib.h>
#include <stdbool.h>
#include "../../dbg.h"
#include "../../mem.h"

#include "node.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for an "for each" statement.
//
// var_decl   - The variable declaration for the loop variable.
// enumerator - The source of the enumeration.
// block      - The block to execute for each item in the enumerator.
// ret        - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_for_each_stmt_create(eql_ast_node *var_decl,
                                 eql_ast_node *enumerator,
                                 eql_ast_node *block,
                                 eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_FOR_EACH_STMT;
    node->parent = NULL;

    // Assign variable declaration
    node->for_each_stmt.var_decl = var_decl;
    if(var_decl != NULL) {
        var_decl->parent = node;
    }

    // Assign enumerator.
    node->for_each_stmt.enumerator = enumerator;
    if(enumerator != NULL) {
        enumerator->parent = node;
    }

    // Assign block
    node->for_each_stmt.block = block;
    if(block != NULL) {
        block->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees an "for each" statement AST node from memory.
//
// node - The AST node to free.
void eql_ast_for_each_stmt_free(struct eql_ast_node *node)
{
    if(node->for_each_stmt.var_decl) eql_ast_node_free(node->for_each_stmt.var_decl);
    node->for_each_stmt.var_decl = NULL;

    if(node->for_each_stmt.enumerator) eql_ast_node_free(node->for_each_stmt.enumerator);
    node->for_each_stmt.enumerator = NULL;

    if(node->for_each_stmt.block) eql_ast_node_free(node->for_each_stmt.block);
    node->for_each_stmt.block = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the "for each" statement AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_for_each_stmt_codegen(eql_ast_node *node, eql_module *module,
                                  LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(node->for_each_stmt.var_decl != NULL, "For Each statement variable declaration required");
    check(node->for_each_stmt.enumerator != NULL, "For Each statement enumerator required");
    check(node->for_each_stmt.block != NULL, "For Each statement block required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Codegen variable declaration.
    LLVMValueRef var_decl_value = NULL;
    rc = eql_ast_node_codegen(node->for_each_stmt.var_decl, module, &var_decl_value);
    check(rc == 0, "Unable to codegen for each loop variable declaration");
    
    // Create a block for the loop.
    LLVMBasicBlockRef loop_block  = LLVMAppendBasicBlock(module->llvm_function, "");
    LLVMBasicBlockRef body_block  = LLVMAppendBasicBlock(module->llvm_function, "");
    LLVMBasicBlockRef exit_block  = LLVMAppendBasicBlock(module->llvm_function, "");

    LLVMBuildBr(builder, loop_block);
    LLVMPositionBuilderAtEnd(builder, loop_block);
    
    // Retrieve enumerator pointer
    LLVMValueRef enumerator_value = NULL;
    rc = eql_ast_node_get_var_pointer(node->for_each_stmt.enumerator, module, &enumerator_value);
    check(rc == 0, "Unable to retrieve for loop enumerator pointer");
    
    // Load the enumerator type.
    bstring enumerator_type_name = NULL;
    rc = eql_ast_node_get_type(node->for_each_stmt.enumerator, module, &enumerator_type_name);
    check(rc == 0 && enumerator_type_name != NULL, "Unable to find enumerator type");

    // Codegen a function call to the eof() method of the enumerator.
    LLVMValueRef *eof_args = malloc(sizeof(LLVMValueRef) * 1);
    eof_args[0] = enumerator_value;
    bstring eof_function_name = bformat("%s___eof", bdata(enumerator_type_name));
    check_mem(eof_function_name);
    LLVMValueRef eof_func = LLVMGetNamedFunction(module->llvm_module, bdata(eof_function_name));
    check(eof_func != NULL, "Unable to find function: %s", bdata(eof_function_name));
    check(LLVMCountParams(eof_func) == 1, "Argument mismatch (got 1, expected %d)", LLVMCountParams(eof_func));
    LLVMValueRef eof_value = LLVMBuildCall(builder, eof_func, eof_args, 1, "");

    // If we receive an eof then exit. Otherwise loop through body.
    LLVMBuildCondBr(builder, eof_value, exit_block, body_block);

    // Move into main body.
    LLVMPositionBuilderAtEnd(builder, body_block);

    // Codegen a function call to the next() method of the enumerator.
    LLVMValueRef *next_args = malloc(sizeof(LLVMValueRef) * 2);
    next_args[0] = enumerator_value;
    next_args[1] = var_decl_value;
    bstring next_function_name = bformat("%s___next", bdata(enumerator_type_name));
    check_mem(next_function_name);
    LLVMValueRef next_func = LLVMGetNamedFunction(module->llvm_module, bdata(next_function_name));
    check(next_func != NULL, "Unable to find function: %s", bdata(next_function_name));
    check(LLVMCountParams(next_func) == 2, "Argument mismatch (got 2, expected %d)", LLVMCountParams(next_func));
    LLVMBuildCall(builder, next_func, next_args, 2, "");
    
    // Generate user-provided loop block.
    rc = eql_ast_block_codegen_with_block(node->for_each_stmt.block, module, body_block);
    check(rc == 0, "Unable to codegen for each statement block");
    
    body_block = LLVMGetInsertBlock(builder);

    // Move to the false block.
    LLVMMoveBasicBlockAfter(exit_block, body_block);
    LLVMPositionBuilderAtEnd(builder, body_block);

    LLVMBuildBr(builder, loop_block);
    LLVMPositionBuilderAtEnd(builder, exit_block);
    
    *value = NULL;
    return 0;

error:
    *value = NULL;
    return -1;
}



//--------------------------------------
// Misc
//--------------------------------------

// Searches for variable declarations within the for each statement.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_for_each_stmt_get_var_decl(eql_ast_node *node, bstring name,
                                       eql_ast_node **var_decl)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_FOR_EACH_STMT, "Node type must be 'for each'");

    // Check against the variable declaration.
    if(biseq(node->for_each_stmt.var_decl->var_decl.name, name)) {
        *var_decl = node->for_each_stmt.var_decl;
    }
    else {
        *var_decl = NULL;
    }

    return 0;
    
error:
    *var_decl = NULL;
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
int eql_ast_for_each_stmt_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<for-each-stmt>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    if(node->for_each_stmt.var_decl != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.var_decl, ret);
        check(rc == 0, "Unable to dump for each statement variable declaration");
    }

    if(node->for_each_stmt.enumerator != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.enumerator, ret);
        check(rc == 0, "Unable to dump for each statement enumerator");
    }

    if(node->for_each_stmt.block != NULL) {
        rc = eql_ast_node_dump(node->for_each_stmt.block, ret);
        check(rc == 0, "Unable to dump for each statement block");
    }

    return 0;

error:
    return -1;
}
