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

// Creates an AST node for a block.
//
// exprs      - An array of expression nodes.
// expr_count - The number of expression nodes in the block.
// ret        - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_create(bstring name, struct eql_ast_node **exprs,
                         unsigned int expr_count,
                         struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_BLOCK;
    node->parent = NULL;
    node->block.name = bstrcpy(name);
    node->block.exprs = NULL;
    node->block.expr_count = 0;
    
    // Add expressions.
    int rc = eql_ast_block_add_exprs(node, exprs, expr_count);
    check(rc == 0, "Unable to add expressions to block");
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a block AST node from memory.
//
// node - The AST node to free.
void eql_ast_block_free(struct eql_ast_node *node)
{
    if(node->block.name) bdestroy(node->block.name);
    node->block.name = NULL;
    
    if(node->block.expr_count > 0) {
        unsigned int i;
        for(i=0; i<node->block.expr_count; i++) {
            eql_ast_node_free(node->block.exprs[i]);
            node->block.exprs[i] = NULL;
        }
        node->block.expr_count = 0;
    }
    free(node->block.exprs);
    node->block.exprs = NULL;
}


//--------------------------------------
// Expression Management
//--------------------------------------

// Appends an expression to the end of the block.
//
// block - The block to append the expression to.
// expr  - The expression to append.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_add_expr(struct eql_ast_node *block, struct eql_ast_node *expr)
{
    // Validate.
    check(block != NULL, "Block is required");
    check(block->type == EQL_AST_TYPE_BLOCK, "Block node is invalid type: %d", block->type);
    check(expr != NULL, "Expression is required");
    
    // Append expression to block.
    block->block.expr_count++;
    block->block.exprs = realloc(block->block.exprs, sizeof(eql_ast_node*) * block->block.expr_count);
    check_mem(block->block.exprs);
    block->block.exprs[block->block.expr_count-1] = expr;
    
    // Assign parent reference to expression.
    expr->parent = block;
    
    return 0;

error:
    return -1;
}

// Appends an multiple expressions to the end of a block.
//
// block      - The block to append the expressions to.
// exprs      - The expression to append.
// expr_count - The number of expression to append.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_add_exprs(struct eql_ast_node *block,
                            struct eql_ast_node **exprs,
                            unsigned int expr_count)
{
    // Validate.
    check(block != NULL, "Block is required");
    check(block->type == EQL_AST_TYPE_BLOCK, "Block node is invalid type: %d", block->type);
    check(exprs != NULL || expr_count == 0, "Expressions required");
    
    // Append expressions to block.
    unsigned int i;
    for(i=0; i<expr_count; i++) {
        int rc = eql_ast_block_add_expr(block, exprs[i]);
        check(rc == 0, "Unable to add expression to block");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Creates the LLVM block for this AST but does not generate the statements.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// block   - A pointer to where the LLVM basic block should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_codegen_block(eql_ast_node *node, eql_module *module, 
                          LLVMBasicBlockRef *block)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Unable to create block without a function");

    // Create LLVM block.
    *block = LLVMAppendBasicBlock(module->llvm_function, bdatae(node->block.name, ""));
    
    return 0;
    
error:
    *block = NULL;
    return -1;
}
    
// Recursively generates LLVM code for the block AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// block   - The LLVM block to insert instructions into.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_codegen_with_block(eql_ast_node *node, eql_module *module, 
                                     LLVMBasicBlockRef block)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Unable to add a block without a function");
    check(block != NULL, "LLVM block required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Move builder to the end of the block.
    LLVMPositionBuilderAtEnd(builder, block);
   
    // Add block scope.
    rc = eql_module_push_scope(module, node);
    check(rc == 0, "Unable to add block scope");

    // Codegen expressions in block.
    unsigned int i;
    for(i=0; i<node->block.expr_count; i++) {
        LLVMValueRef expression_value;
        int rc = eql_ast_node_codegen(node->block.exprs[i], module, &expression_value);
        check(rc == 0, "Unable to codegen block expression");
    }

    // Remove block scope.
    rc = eql_module_pop_scope(module, node);
    check(rc == 0, "Unable to remove block scope");

    return 0;

error:
    return -1;
}

// Recursively generates LLVM code for the block AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_codegen(eql_ast_node *node, eql_module *module, 
                          LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Unable to add a block without a function");

    // Generate block.
    LLVMBasicBlockRef block = NULL;
    rc = eql_ast_block_codegen_block(node, module, &block);
    check(rc == 0, "Unable to create LLVM block");
    
    // Generate statements within block.
    rc = eql_ast_block_codegen_with_block(node, module, block);
    check(rc == 0, "Unable to codegen block statements");

    *value = LLVMBasicBlockAsValue(block);
    return 0;

error:
    *value = NULL;
    return -1;
}

//--------------------------------------
// Misc
//--------------------------------------

// Searches for variable declarations within the block.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_block_get_var_decl(eql_ast_node *node, bstring name,
                               eql_ast_node **var_decl)
{
    unsigned int i;

    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BLOCK, "Node type must be 'block'");

    // Search expressions for variable declaration.
    *var_decl = NULL;
    for(i=0; i<node->block.expr_count; i++) {
        if(node->block.exprs[i]->type == EQL_AST_TYPE_VAR_DECL) {
            if(biseq(node->block.exprs[i]->var_decl.name, name)) {
                *var_decl = node->block.exprs[i];
                break;
            }
        }
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
int eql_ast_block_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<block name='%s'>\n", bdatae(node->block.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    unsigned int i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = eql_ast_node_dump(node->block.exprs[i], ret);
        check(rc == 0, "Unable to dump block expression");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
