#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

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

// Creates an AST node for an "if" statement.
//
// ret        - A pointer to where the ast node will be returned.
//
// Returns an if statement node.
eql_ast_node *eql_ast_if_stmt_create()
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_IF_STMT;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->if_stmt.conditions = NULL;
    node->if_stmt.blocks = NULL;
    node->if_stmt.block_count = 0;
    node->if_stmt.else_block = NULL;

    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees an "if" statement AST node from memory.
//
// node - The AST node to free.
void eql_ast_if_stmt_free(struct eql_ast_node *node)
{
    unsigned int i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        eql_ast_node *condition = node->if_stmt.conditions[i];
        eql_ast_node *block = node->if_stmt.blocks[i];
        
        if(condition) eql_ast_node_free(condition);
        node->if_stmt.conditions[i] = NULL;

        if(block) eql_ast_node_free(block);
        node->if_stmt.blocks[i] = NULL;
    }
    
    if(node->if_stmt.conditions) free(node->if_stmt.conditions);
    node->if_stmt.conditions = NULL;

    if(node->if_stmt.blocks) free(node->if_stmt.blocks);
    node->if_stmt.blocks = NULL;

    if(node->if_stmt.else_block) eql_ast_node_free(node->if_stmt.else_block);
    node->if_stmt.else_block = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_copy(eql_ast_node *node, eql_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_if_stmt_create();
    check_mem(clone);

    // Copy conditions & blocks.
    clone->if_stmt.block_count = node->if_stmt.block_count;
    clone->if_stmt.conditions = calloc(clone->if_stmt.block_count, sizeof(*clone->if_stmt.conditions));
    check_mem(clone->if_stmt.conditions);
    clone->if_stmt.blocks = calloc(clone->if_stmt.block_count, sizeof(*clone->if_stmt.blocks));
    check_mem(clone->if_stmt.blocks);
    for(i=0; i<clone->if_stmt.block_count; i++) {
        rc = eql_ast_node_copy(node->if_stmt.conditions[i], &clone->if_stmt.conditions[i]);
        check(rc == 0, "Unable to copy condition");
        if(clone->if_stmt.conditions[i]) clone->if_stmt.conditions[i]->parent = clone;

        rc = eql_ast_node_copy(node->if_stmt.blocks[i], &clone->if_stmt.blocks[i]);
        check(rc == 0, "Unable to copy block");
        if(clone->if_stmt.blocks[i]) clone->if_stmt.blocks[i]->parent = clone;
    }
    
    // Copy else block
    rc = eql_ast_node_copy(node->if_stmt.else_block, &clone->if_stmt.else_block);
    check(rc == 0, "Unable to copy else block");
    if(clone->if_stmt.else_block) clone->if_stmt.else_block->parent = clone;
    
    *ret = clone;
    return 0;

error:
    eql_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Block Management
//--------------------------------------

// Adds a block with an associated condition to the node.
//
// node      - The if statement AST node.
// condition - The condition to execute the block.
// block     - The block executed if the condition is true.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_add_block(eql_ast_node *node, eql_ast_node *condition,
                              eql_ast_node *block)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_IF_STMT, "Unexpected node type: %d", node->type);
    check(condition != NULL, "Condition required");
    check(block != NULL, "Block required");

    // Increment block count.
    node->if_stmt.block_count++;

    // Append condition.
    node->if_stmt.conditions = realloc(node->if_stmt.conditions, sizeof(eql_ast_node*) * node->if_stmt.block_count);
    check_mem(node->if_stmt.conditions);
    node->if_stmt.conditions[node->if_stmt.block_count-1] = condition;
    
    // Append block.
    node->if_stmt.blocks = realloc(node->if_stmt.blocks, sizeof(eql_ast_node*) * node->if_stmt.block_count);
    check_mem(node->if_stmt.blocks);
    node->if_stmt.blocks[node->if_stmt.block_count-1] = block;
    
    // Link condition and block.
    condition->parent = node;
    block->parent = node;
    
    return 0;

error:
    return -1;
}

// Adds a list of blocks and related conditions to an if statement AST node.
//
// node       - The node to add the blocks to.
// conditions - A list of conditions to add.
// blocks     - A list of members to add.
// count      - The number of members to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_add_blocks(eql_ast_node *node,
                               eql_ast_node **conditions,
                               eql_ast_node **blocks,
                               unsigned int count)
{
    // Validate.
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_IF_STMT, "Unexpected node type: %d", node->type);
    check(conditions != NULL || count == 0, "Conditions required");
    check(blocks != NULL || count == 0, "Blocks required");

    // Add each block.
    unsigned int i;
    for(i=0; i<count; i++) {
        int rc = eql_ast_if_stmt_add_block(node, conditions[i], blocks[i]);
        check(rc == 0, "Unable to add block to node");
    }
    
    return 0;

error:
    return -1;
}

// Assigns a block to the else condition of an if statement.
//
// node - The if statement AST node.
// block - The else block.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_set_else_block(eql_ast_node *node, eql_ast_node *block)
{
    check(node != NULL, "Node required");
    
    // Assign else block.
    node->if_stmt.else_block = block;
    if(block != NULL) {
        block->parent = node;
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Generates an if/else block. The "else if" blocks are recusively nested
// so that the IR generation matches what LLVM expects.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// index   - The block number to generate for.
//
// Returns 0 if successful, otherwise returns -1.
int codegen_block(eql_ast_node *node, eql_module *module, unsigned int index)
{
    int rc;
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;
 
    // Retrieve the associated condition & block.
    eql_ast_node *condition = node->if_stmt.conditions[index];
    eql_ast_node *block = node->if_stmt.blocks[index];

    // Generate IR for condition.
    LLVMValueRef condition_value  = NULL;
    rc = eql_ast_node_codegen(condition, module, &condition_value);

    // Generate blocks.
    bool has_alt_block = (index+1 < node->if_stmt.block_count || node->if_stmt.else_block != NULL);
    LLVMBasicBlockRef true_block  = LLVMAppendBasicBlock(module->llvm_function, "");
    LLVMBasicBlockRef false_block = LLVMAppendBasicBlock(module->llvm_function, "");
    
    // Create a conditional branch.
    LLVMBuildCondBr(builder, condition_value, true_block, false_block);
    
    // Codegen the true block body.
    rc = eql_ast_block_codegen_with_block(block, module, true_block);
    check(rc == 0, "Unable to codegen if statement true block body");
    
    // Retrieve last block (in case this is nested).
    true_block = LLVMGetInsertBlock(builder);

    // Move to the false block.
    LLVMMoveBasicBlockAfter(false_block, true_block);
    LLVMPositionBuilderAtEnd(builder, false_block);

    // Codegen "else if" blocks.
    if(index+1 < node->if_stmt.block_count) {
        codegen_block(node, module, index+1);
        false_block = LLVMGetInsertBlock(builder);
    }
    // If there are no more "else if" blocks then codegen the "else" block
    else if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_block_codegen_with_block(node->if_stmt.else_block, module, false_block);
        check(rc == 0, "Unable to codegen if statement else block");
        false_block = LLVMGetInsertBlock(builder);
    }

    // Merge blocks together.
    LLVMBasicBlockRef merge_block = NULL;
    if(has_alt_block) {
        merge_block = LLVMAppendBasicBlock(module->llvm_function, "");
    }
    else {
        merge_block = false_block;
    }
    
    LLVMPositionBuilderAtEnd(builder, true_block);
    LLVMBuildBr(builder, merge_block);
    if(has_alt_block) {
        LLVMPositionBuilderAtEnd(builder, false_block);
        LLVMBuildBr(builder, merge_block);
    }
    LLVMPositionBuilderAtEnd(builder, merge_block);

    return 0;
    
error:
    return -1;
}

// Recursively generates LLVM code for the "if" statement AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_codegen(eql_ast_node *node, eql_module *module,
                            LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(node->if_stmt.block_count > 0, "If statement blocks required");

    // Recursively generate and wrap blocks.
    rc = codegen_block(node, module, 0);
    check(rc == 0, "Unable to generate if block");

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
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_preprocess(eql_ast_node *node, eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Preprocess arguments.
    uint32_t i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        rc = eql_ast_node_preprocess(node->if_stmt.conditions[i], module);
        check(rc == 0, "Unable to preprocess if statement condition");

        rc = eql_ast_node_preprocess(node->if_stmt.blocks[i], module);
        check(rc == 0, "Unable to preprocess if statement block");
    }
    
    // Preprocess else block.
    if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_node_preprocess(node->if_stmt.else_block, module);
        check(rc == 0, "Unable to preprocess if statement else block");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type references used by a node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_get_type_refs(eql_ast_node *node,
                                  eql_ast_node ***type_refs,
                                  uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add if & else if type refs.
    uint32_t i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        rc = eql_ast_node_get_type_refs(node->if_stmt.blocks[i], type_refs, count);
        check(rc == 0, "Unable to add if block type refs");
    }

    // Compute else block type refs.
    if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_node_get_type_refs(node->if_stmt.else_block, type_refs, count);
        check(rc == 0, "Unable to add else block type refs");
    }

    return 0;
    
error:
    eql_ast_node_type_refs_free(type_refs, count);
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
int eql_ast_if_stmt_get_dependencies(eql_ast_node *node,
                                     bstring **dependencies,
                                     uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Add if & else if dependencies.
    uint32_t i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        rc = eql_ast_node_get_dependencies(node->if_stmt.blocks[i], dependencies, count);
        check(rc == 0, "Unable to add if block dependencies");
    }

    // Compute else block dependencies.
    if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_node_get_dependencies(node->if_stmt.else_block, dependencies, count);
        check(rc == 0, "Unable to add else block dependencies");
    }

    return 0;
    
error:
    eql_ast_node_dependencies_free(dependencies, count);
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
int eql_ast_if_stmt_validate(eql_ast_node *node, eql_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate arguments.
    uint32_t i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        rc = eql_ast_node_validate(node->if_stmt.conditions[i], module);
        check(rc == 0, "Unable to validate if statement condition");

        rc = eql_ast_node_validate(node->if_stmt.blocks[i], module);
        check(rc == 0, "Unable to validate if statement block");
    }
    
    // Validate else block.
    if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_node_validate(node->if_stmt.else_block, module);
        check(rc == 0, "Unable to validate if statement else block");
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
int eql_ast_if_stmt_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<if-stmt>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    unsigned int i;
    for(i=0; i<node->if_stmt.block_count; i++) {
        rc = eql_ast_node_dump(node->if_stmt.conditions[i], ret);
        check(rc == 0, "Unable to dump if statement condition");

        rc = eql_ast_node_dump(node->if_stmt.blocks[i], ret);
        check(rc == 0, "Unable to dump if statement block");
    }

    if(node->if_stmt.else_block != NULL) {
        rc = eql_ast_node_dump(node->if_stmt.else_block, ret);
        check(rc == 0, "Unable to dump if statement else block");
    }

    return 0;

error:
    return -1;
}
