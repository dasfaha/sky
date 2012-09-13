#include <stdlib.h>
#include "dbg.h"

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
// Returns a block node.
qip_ast_node *qip_ast_block_create(bstring name, struct qip_ast_node **exprs,
                                   unsigned int expr_count)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_BLOCK;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->block.name = bstrcpy(name);
    node->block.exprs = NULL;
    node->block.expr_count = 0;
    
    // Add expressions.
    int rc = qip_ast_block_add_exprs(node, exprs, expr_count);
    check(rc == 0, "Unable to add expressions to block");
    
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a block AST node from memory.
//
// node - The AST node to free.
void qip_ast_block_free(struct qip_ast_node *node)
{
    if(node != NULL) {
        if(node->block.name) bdestroy(node->block.name);
        node->block.name = NULL;

        qip_ast_block_free_exprs(node);
    }
}

// Frees a the expressions on a block.
//
// node - The node.
void qip_ast_block_free_exprs(qip_ast_node *node)
{
    if(node != NULL) {
        unsigned int i;
        for(i=0; i<node->block.expr_count; i++) {
            qip_ast_node_free(node->block.exprs[i]);
            node->block.exprs[i] = NULL;
        }
        node->block.expr_count = 0;

        free(node->block.exprs);
        node->block.exprs = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_block_create(node->block.name, NULL, 0);
    check_mem(clone);

    // Copy args.
    clone->block.expr_count = node->block.expr_count;
    clone->block.exprs = calloc(clone->block.expr_count, sizeof(*clone->block.exprs));
    check_mem(clone->block.exprs);
    for(i=0; i<clone->block.expr_count; i++) {
        rc = qip_ast_node_copy(node->block.exprs[i], &clone->block.exprs[i]);
        check(rc == 0, "Unable to copy expr");
        if(clone->block.exprs[i]) clone->block.exprs[i]->parent = clone;
    }
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Expression Management
//--------------------------------------

// Retrieves the index of the expression on the block.
//
// node - The node.
// expr - The expression.
// ret  - A pointer to where the index should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_get_expr_index(qip_ast_node *node, qip_ast_node *expr,
                                 int32_t *ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(expr != NULL, "Expression required");
    check(ret != NULL, "Return pointer required");

    // Initialize return value.
    *ret = -1;

    // Search for expression and remove.
    unsigned int i;
    for(i=0; i<node->block.expr_count; i++) {
        if(node->block.exprs[i] == expr) {
            *ret = i;
            break;
        }
    }

    return 0;

error:
    *ret = -1;
    return -1;
}

// Appends an expression to the end of the block.
//
// block - The block to append the expression to.
// expr  - The expression to append.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_add_expr(struct qip_ast_node *block, struct qip_ast_node *expr)
{
    // Validate.
    check(block != NULL, "Block is required");
    check(block->type == QIP_AST_TYPE_BLOCK, "Block node is invalid type: %d", block->type);
    check(expr != NULL, "Expression is required");
    
    // Append expression to block.
    block->block.expr_count++;
    block->block.exprs = realloc(block->block.exprs, sizeof(qip_ast_node*) * block->block.expr_count);
    check_mem(block->block.exprs);
    block->block.exprs[block->block.expr_count-1] = expr;
    
    // Assign parent reference to expression.
    expr->parent = block;
    
    return 0;

error:
    return -1;
}

// Inserts an expression at the given index.
//
// block - The block.
// expr  - The expression.
// index - The index to insert at.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_insert_expr(qip_ast_node *node, qip_ast_node *expr,
                              unsigned int index)
{
    check(node != NULL, "Block required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(expr != NULL, "Expression required");
    
    // Resize array.
    node->block.expr_count++;
    node->block.exprs = realloc(node->block.exprs, sizeof(*node->block.exprs) * node->block.expr_count);
    check_mem(node->block.exprs);

    // Shift everything down.
    unsigned int i;
    for(i=node->block.expr_count-1; i>index; i--) {
        node->block.exprs[i] = node->block.exprs[i-1];
    }

    // Assign parent reference to expression.
    node->block.exprs[index] = expr;
    expr->parent = node;
    
    return 0;

error:
    return -1;
}

// Prepends an expression to the beginning of the block.
//
// block - The block.
// expr  - The expression.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_prepend_expr(struct qip_ast_node *block, struct qip_ast_node *expr)
{
    // Validate.
    check(block != NULL, "Block required");
    check(block->type == QIP_AST_TYPE_BLOCK, "Block node is invalid type: %d", block->type);
    check(expr != NULL, "Expression is required");
    
    // Append expression to block.
    block->block.expr_count++;
    block->block.exprs = realloc(block->block.exprs, sizeof(qip_ast_node*) * block->block.expr_count);
    check_mem(block->block.exprs);

    // Shift everything down.
    unsigned int i;
    for(i=block->block.expr_count-1; i>0; i--) {
        block->block.exprs[i] = block->block.exprs[i-1];
    }

    // Assign parent reference to expression.
    block->block.exprs[0] = expr;
    expr->parent = block;
    
    return 0;

error:
    return -1;
}

// Removes an expression from the block.
//
// node - The node.
// expr - The expression to remove.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_remove_expr(qip_ast_node *node, qip_ast_node *expr)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(expr != NULL, "Expression required");

    // Clear expression parent if it matches block.
    if(expr->parent == node) {
        expr->parent = NULL; 
    }

    // Search for expression and remove.
    unsigned int i, j;
    for(i=0; i<node->block.expr_count; i++) {
        if(node->block.exprs[i] == expr) {
            node->block.expr_count--;
            for(j=i; j<node->block.expr_count; j++) {
                node->block.exprs[j] = node->block.exprs[j+1];
            }
        }
    }
    
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
int qip_ast_block_add_exprs(struct qip_ast_node *block,
                            struct qip_ast_node **exprs,
                            unsigned int expr_count)
{
    // Validate.
    check(block != NULL, "Block is required");
    check(block->type == QIP_AST_TYPE_BLOCK, "Block node is invalid type: %d", block->type);
    check(exprs != NULL || expr_count == 0, "Expressions required");
    
    // Append expressions to block.
    unsigned int i;
    for(i=0; i<expr_count; i++) {
        int rc = qip_ast_block_add_expr(block, exprs[i]);
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
int qip_ast_block_codegen_block(qip_ast_node *node, qip_module *module, 
                                LLVMBasicBlockRef *block)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");
    
    // Retrieve current function scope.
    qip_scope *scope = NULL;
    rc = qip_module_get_current_function_scope(module, &scope);
    check(rc == 0 && scope != NULL, "Unable to retrieve current function scope");

    // Create LLVM block.
    *block = LLVMAppendBasicBlock(scope->llvm_function, bdatae(node->block.name, ""));
    
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
int qip_ast_block_codegen_with_block(qip_ast_node *node, qip_module *module, 
                                     LLVMBasicBlockRef block)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");
    check(block != NULL, "LLVM block required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Move builder to the end of the block.
    LLVMPositionBuilderAtEnd(builder, block);
   
    // Add block scope.
    qip_scope *scope = qip_scope_create_block(); check_mem(scope);
    rc = qip_module_push_scope(module, scope);
    check(rc == 0, "Unable to add block scope");

    // Codegen expressions in block.
    unsigned int i;
    for(i=0; i<node->block.expr_count; i++) {
        LLVMValueRef expression_value;
        int rc = qip_ast_node_codegen(node->block.exprs[i], module, &expression_value);
        check(rc == 0, "Unable to codegen block expression");
    }

    // Remove block scope.
    rc = qip_module_pop_scope(module);
    check(rc == 0, "Unable to remove block scope");

    return 0;

error:
    return -1;
}

// Generates deconstructor calls for all variable declarations in block.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_codegen_destroy(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");

    // Generate calls to deconstructors for each variable declaration.
    unsigned int i;
    for(i=0; i<node->block.expr_count; i++) {
        if(node->block.exprs[i]->type == QIP_AST_TYPE_VAR_DECL) {
            LLVMValueRef value;
            rc = qip_ast_var_decl_codegen_destroy(node->block.exprs[i], module, &value);
            check(rc == 0, "Unable to codegen variable declaration destroy");
        }
    }
    
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
int qip_ast_block_codegen(qip_ast_node *node, qip_module *module, 
                          LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type expected to be 'block'");
    check(node->parent != NULL, "Node parent required");
    check(module != NULL, "Module required");

    // Generate block.
    LLVMBasicBlockRef block = NULL;
    rc = qip_ast_block_codegen_block(node, module, &block);
    check(rc == 0, "Unable to create LLVM block");
    
    // Generate statements within block.
    rc = qip_ast_block_codegen_with_block(node, module, block);
    check(rc == 0, "Unable to codegen block statements");

    *value = LLVMBasicBlockAsValue(block);
    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocess the node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_preprocess(qip_ast_node *node, qip_module *module,
                             qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess expressions.
    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_preprocess(node->block.exprs[i], module, stage);
        check(rc == 0, "Unable to preprocess block expression");
    }

    return 0;

error:
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
int qip_ast_block_get_var_decl(qip_ast_node *node, bstring name,
                               qip_ast_node **var_decl)
{
    unsigned int i;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_BLOCK, "Node type must be 'block'");

    // Search expressions for variable declaration.
    *var_decl = NULL;
    for(i=0; i<node->block.expr_count; i++) {
        if(node->block.exprs[i]->type == QIP_AST_TYPE_VAR_DECL) {
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
// Find
//--------------------------------------

// Computes a list of type references used by the node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_get_type_refs(qip_ast_node *node,
                                qip_ast_node ***type_refs,
                                uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Recursively retrieve type refs for expressions.
    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_get_type_refs(node->block.exprs[i], type_refs, count);
        check(rc == 0, "Unable to add block expression type refs");
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
int qip_ast_block_get_var_refs(qip_ast_node *node, bstring name,
                               qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_get_var_refs(node->block.exprs[i], name, array);
        check(rc == 0, "Unable to add block expression var refs");
    }

    return 0;
    
error:
    return -1;
}

// Retrieves all variable reference of a given type name within this node.
//
// node      - The node.
// module    - The module.
// type_name - The type name.
// array     - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_block_get_var_refs_by_type(qip_ast_node *node, qip_module *module,
                                       bstring type_name, qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(type_name != NULL, "Type name required");
    check(array != NULL, "Array required");

    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_get_var_refs_by_type(node->block.exprs[i], module, type_name, array);
        check(rc == 0, "Unable to add block expression var refs by type");
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
int qip_ast_block_get_dependencies(qip_ast_node *node,
                                   bstring **dependencies,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Recursively retrieve dependencies for expressions.
    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_get_dependencies(node->block.exprs[i], dependencies, count);
        check(rc == 0, "Unable to add block expression dependencies");
    }

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
int qip_ast_block_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Validate expressions.
    uint32_t i;
    for(i=0; i<node->block.expr_count; i++) {
        rc = qip_ast_node_validate(node->block.exprs[i], module);
        check(rc == 0, "Unable to validate block expression");
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
int qip_ast_block_dump(qip_ast_node *node, bstring ret)
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
        rc = qip_ast_node_dump(node->block.exprs[i], ret);
        check(rc == 0, "Unable to dump block expression");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
