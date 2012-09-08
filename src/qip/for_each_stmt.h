#ifndef _qip_ast_for_each_stmt_h
#define _qip_ast_for_each_stmt_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an "if" statement in the AST.
typedef struct {
    qip_ast_node *var_decl;
    qip_ast_node *enumerator;
    qip_ast_node *block;
} qip_ast_for_each_stmt;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_for_each_stmt_create(qip_ast_node *var_decl,
    qip_ast_node *enumerator, qip_ast_node *block);

void qip_ast_for_each_stmt_free(struct qip_ast_node *node);

int qip_ast_for_each_stmt_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_for_each_stmt_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_for_each_stmt_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_for_each_stmt_get_var_decl(qip_ast_node *node, bstring name,
    qip_ast_node **var_decl);

int qip_ast_for_each_stmt_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_for_each_stmt_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_for_each_stmt_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_for_each_stmt_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_for_each_stmt_dump(qip_ast_node *node, bstring ret);

#endif