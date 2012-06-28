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

// Creates an AST node for an "if" statement.
//
// ret        - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_if_stmt_create(eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_IF_STMT;
    node->parent = NULL;

    node->if_stmt.conditions = NULL;
    node->if_stmt.blocks = NULL;
    node->if_stmt.block_count = 0;
    node->if_stmt.else_block = NULL;

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
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
