#ifndef _qip_ast_array_literal_h
#define _qip_ast_array_literal_h

#include <stdbool.h>

#include "module.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal array in the AST.
typedef struct {
    qip_ast_node **items;
    unsigned int item_count;
    qip_ast_node *type_ref;
} qip_ast_array_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_array_literal_create();

void qip_ast_array_literal_free(qip_ast_node *node);

void qip_ast_array_literal_free_items(qip_ast_node *node);

int qip_ast_array_literal_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Item Management
//--------------------------------------

int qip_ast_array_literal_add_item(qip_ast_node *node, qip_ast_node *item);

int qip_ast_array_literal_add_items(qip_ast_node *block,
    qip_ast_node **exprs, unsigned int expr_count);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_array_literal_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_array_literal_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_array_literal_get_type(qip_ast_node *node,
    qip_ast_node **type_ref);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_array_literal_dump(qip_ast_node *node, bstring ret);

#endif