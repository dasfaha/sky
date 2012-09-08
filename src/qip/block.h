#ifndef _qip_ast_block_h
#define _qip_ast_block_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

typedef struct qip_ast_block qip_ast_block;

// Represents a block in the AST.
struct qip_ast_block {
    bstring name;
    struct qip_ast_node **exprs;
    unsigned int expr_count;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_block_create(bstring name, qip_ast_node **exprs,
    unsigned int expr_count);

void qip_ast_block_free(qip_ast_node *node);

void qip_ast_block_free_exprs(qip_ast_node *node);

int qip_ast_block_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Expression Management
//--------------------------------------

int qip_ast_block_get_expr_index(qip_ast_node *node, qip_ast_node *expr,
    int32_t *ret);

int qip_ast_block_add_expr(qip_ast_node *block, qip_ast_node *expr);

int qip_ast_block_insert_expr(qip_ast_node *node, qip_ast_node *expr,
    unsigned int index);

int qip_ast_block_prepend_expr(qip_ast_node *block, qip_ast_node *expr);

int qip_ast_block_remove_expr(qip_ast_node *block, qip_ast_node *expr);

int qip_ast_block_add_exprs(qip_ast_node *block,
    qip_ast_node **exprs, unsigned int expr_count);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_block_codegen_block(qip_ast_node *node, qip_module *module,
    LLVMBasicBlockRef *block);

int qip_ast_block_codegen_with_block(qip_ast_node *node, qip_module *module,
    LLVMBasicBlockRef block);

int qip_ast_block_codegen_destroy(qip_ast_node *node, qip_module *module);

int qip_ast_block_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_block_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_block_get_var_decl(qip_ast_node *node, bstring name,
    qip_ast_node **var_decl);

int qip_ast_block_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_block_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_block_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_block_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_block_dump(qip_ast_node *node, bstring ret);

#endif