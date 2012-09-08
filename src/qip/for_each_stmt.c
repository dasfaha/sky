#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"
#include "util.h"

#include "node.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_for_each_stmt_validate_enumerable_enumerator(qip_ast_node *node,
    qip_module *module);


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
// Returns a for each statement node.
qip_ast_node *qip_ast_for_each_stmt_create(qip_ast_node *var_decl,
                                           qip_ast_node *enumerator,
                                           qip_ast_node *block)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_FOR_EACH_STMT;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

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

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees an "for each" statement AST node from memory.
//
// node - The AST node to free.
void qip_ast_for_each_stmt_free(struct qip_ast_node *node)
{
    if(node->for_each_stmt.var_decl) qip_ast_node_free(node->for_each_stmt.var_decl);
    node->for_each_stmt.var_decl = NULL;

    if(node->for_each_stmt.enumerator) qip_ast_node_free(node->for_each_stmt.enumerator);
    node->for_each_stmt.enumerator = NULL;

    if(node->for_each_stmt.block) qip_ast_node_free(node->for_each_stmt.block);
    node->for_each_stmt.block = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_for_each_stmt_create(NULL, NULL, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->for_each_stmt.var_decl, &clone->for_each_stmt.var_decl);
    check(rc == 0, "Unable to copy var decl");
    if(clone->for_each_stmt.var_decl) clone->for_each_stmt.var_decl->parent = clone;
    
    rc = qip_ast_node_copy(node->for_each_stmt.enumerator, &clone->for_each_stmt.enumerator);
    check(rc == 0, "Unable to copy enumerator");
    if(clone->for_each_stmt.enumerator) clone->for_each_stmt.enumerator->parent = clone;
    
    rc = qip_ast_node_copy(node->for_each_stmt.block, &clone->for_each_stmt.block);
    check(rc == 0, "Unable to copy block");
    if(clone->for_each_stmt.block) clone->for_each_stmt.block->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
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
int qip_ast_for_each_stmt_codegen(qip_ast_node *node, qip_module *module,
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
    rc = qip_ast_node_codegen(node->for_each_stmt.var_decl, module, &var_decl_value);
    check(rc == 0, "Unable to codegen for each loop variable declaration");
    
    // Retrieve current function.
    qip_scope *scope = NULL;
    rc = qip_module_get_current_function_scope(module, &scope);
    check(rc == 0 && scope != NULL, "Unable to retrieve current function scope");

    // Create a block for the loop.
    LLVMBasicBlockRef loop_block  = LLVMAppendBasicBlock(scope->llvm_function, "");
    LLVMBasicBlockRef body_block  = LLVMAppendBasicBlock(scope->llvm_function, "");
    LLVMBasicBlockRef exit_block  = LLVMAppendBasicBlock(scope->llvm_function, "");

    LLVMBuildBr(builder, loop_block);
    LLVMPositionBuilderAtEnd(builder, loop_block);
    
    // Retrieve enumerator pointer
    LLVMValueRef enumerator_value = NULL;
    rc = qip_ast_node_get_var_pointer(node->for_each_stmt.enumerator, module, &enumerator_value);
    check(rc == 0, "Unable to retrieve for loop enumerator pointer");
    
    // Load the enumerator type.
    bstring enumerator_type_name = NULL;
    rc = qip_ast_node_get_type_name(node->for_each_stmt.enumerator, module, &enumerator_type_name);
    check(rc == 0 && enumerator_type_name != NULL, "Unable to find enumerator type");

    // Codegen a function call to the eof() method of the enumerator.
    LLVMValueRef *eof_args = malloc(sizeof(LLVMValueRef) * 1);
    eof_args[0] = LLVMBuildLoad(builder, enumerator_value, "");
    bstring eof_function_name = bformat("%s.eof", bdata(enumerator_type_name));
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
    next_args[0] = LLVMBuildLoad(builder, enumerator_value, "");
    next_args[1] = LLVMBuildLoad(builder, var_decl_value, "");
    bstring next_function_name = bformat("%s.next", bdata(enumerator_type_name));
    check_mem(next_function_name);
    LLVMValueRef next_func = LLVMGetNamedFunction(module->llvm_module, bdata(next_function_name));
    check(next_func != NULL, "Unable to find function: %s", bdata(next_function_name));
    check(LLVMCountParams(next_func) == 2, "Argument mismatch (got 2, expected %d)", LLVMCountParams(next_func));
    LLVMBuildCall(builder, next_func, next_args, 2, "");
    
    // Generate user-provided loop block.
    rc = qip_ast_block_codegen_with_block(node->for_each_stmt.block, module, body_block);
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
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_preprocess(qip_ast_node *node, qip_module *module,
                                     qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Preprocess variable declaration.
    if(node->for_each_stmt.var_decl != NULL) {
        rc = qip_ast_node_preprocess(node->for_each_stmt.var_decl, module, stage);
        check(rc == 0, "Unable to preprocess for each variable declaration");
    }
    
    // Preprocess enumerator.
    if(node->for_each_stmt.enumerator != NULL) {
        rc = qip_ast_node_preprocess(node->for_each_stmt.enumerator, module, stage);
        check(rc == 0, "Unable to preprocess for each statement enumerator");
    }
    
    // Preprocess block.
    if(node->for_each_stmt.block != NULL) {
        rc = qip_ast_node_preprocess(node->for_each_stmt.block, module, stage);
        check(rc == 0, "Unable to preprocess for each statement block");
    }
    
    return 0;

error:
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
int qip_ast_for_each_stmt_get_var_decl(qip_ast_node *node, bstring name,
                                       qip_ast_node **var_decl)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FOR_EACH_STMT, "Node type must be 'for each'");

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
// Find
//--------------------------------------

// Computes a list of type references used by the node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_get_type_refs(qip_ast_node *node,
                                        qip_ast_node ***type_refs,
                                        uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Variable declaration.
    if(node->for_each_stmt.var_decl) {
        rc = qip_ast_node_get_type_refs(node->for_each_stmt.var_decl, type_refs, count);
        check(rc == 0, "Unable to add variable declaration type refs");
    }

    // Block type refs.
    if(node->for_each_stmt.block) {
        rc = qip_ast_node_get_type_refs(node->for_each_stmt.block, type_refs, count);
        check(rc == 0, "Unable to add block type_refs");
    }

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_get_var_refs(qip_ast_node *node, bstring name,
                                       qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    if(node->for_each_stmt.var_decl) {
        rc = qip_ast_node_get_var_refs(node->for_each_stmt.var_decl, name, array);
        check(rc == 0, "Unable to add variable declaration var refs");
    }

    if(node->for_each_stmt.block) {
        rc = qip_ast_node_get_var_refs(node->for_each_stmt.block, name, array);
        check(rc == 0, "Unable to add block var refs");
    }

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Dependencies
//--------------------------------------

// Computes a list of class names that this AST node depends on.
//
// node         - The node to compute dependencies for.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_get_dependencies(qip_ast_node *node,
                                           bstring **dependencies,
                                           uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Variable declaration dependency.
    rc = qip_ast_node_get_dependencies(node->for_each_stmt.var_decl, dependencies, count);
    check(rc == 0, "Unable to add variable declaration dependency");

    // Block dependencies.
    rc = qip_ast_node_get_dependencies(node->for_each_stmt.block, dependencies, count);
    check(rc == 0, "Unable to add block dependencies");

    return 0;
    
error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Validates the AST node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate that class of enumerator is enumerable.
    rc = qip_ast_for_each_stmt_validate_enumerable_enumerator(node, module);
    check(rc == 0, "Unable to validate enumerable enumerator");

    // Validate variable declaration.
    if(node->for_each_stmt.var_decl != NULL) {
        rc = qip_ast_node_validate(node->for_each_stmt.var_decl, module);
        check(rc == 0, "Unable to validate for each variable declaration");
    }
    
    // Validate enumerator.
    if(node->for_each_stmt.enumerator != NULL) {
        rc = qip_ast_node_validate(node->for_each_stmt.enumerator, module);
        check(rc == 0, "Unable to validate for each statement enumerator");
    }
    
    // Validate block.
    if(node->for_each_stmt.block != NULL) {
        rc = qip_ast_node_validate(node->for_each_stmt.block, module);
        check(rc == 0, "Unable to validate for each statement block");
    }
    
    return 0;

error:
    return -1;
}

// Validates that the loop's enumerator is enumerable.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_for_each_stmt_validate_enumerable_enumerator(qip_ast_node *node,
                                                         qip_module *module)
{
    int rc;
    bstring msg;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    struct tagbstring enumerable_metadata_name = bsStatic("Enumerable");
    
    // Load the enumerator type.
    qip_ast_node *enumerator_type = NULL;
    rc = qip_ast_node_get_type(node->for_each_stmt.enumerator, module, &enumerator_type);
    check(rc == 0 && enumerator_type != NULL, "Unable to find enumerator type");

    // Make sure it's not a built in.
    if(qip_is_builtin_type(enumerator_type)) {
        msg = bformat("Unable to enumerator over %s", bdata(enumerator_type->type_ref.name));
    }
    
    // Load class and check if it is enumerable.
    if(!msg) {
        // Retrieve class from module.
        qip_ast_node *class = NULL;
        rc = qip_module_get_ast_class(module, enumerator_type->type_ref.name, &class);
        check(rc == 0, "Unable to retrieve class from module");
        
        // Retrieve enumerable metadata from class.
        qip_ast_node *enumerable_metadata = NULL;
        rc = qip_ast_class_get_metadata_node(class, &enumerable_metadata_name, &enumerable_metadata);
        check(rc == 0, "Unable to retrieve enumerable metadata node");
        
        // If class is not enumerable then add an error message.
        if(enumerable_metadata == NULL) {
            msg = bformat("For-each loop using enumerator whose class '%s' is not enumerable", bdata(enumerator_type->type_ref.name));
        }
    }
    
    // Add an error.
    if(msg != NULL) {
        rc = qip_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
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
int qip_ast_for_each_stmt_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<for-each-stmt>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    if(node->for_each_stmt.var_decl != NULL) {
        rc = qip_ast_node_dump(node->for_each_stmt.var_decl, ret);
        check(rc == 0, "Unable to dump for each statement variable declaration");
    }

    if(node->for_each_stmt.enumerator != NULL) {
        rc = qip_ast_node_dump(node->for_each_stmt.enumerator, ret);
        check(rc == 0, "Unable to dump for each statement enumerator");
    }

    if(node->for_each_stmt.block != NULL) {
        rc = qip_ast_node_dump(node->for_each_stmt.block, ret);
        check(rc == 0, "Unable to dump for each statement block");
    }

    return 0;

error:
    return -1;
}
