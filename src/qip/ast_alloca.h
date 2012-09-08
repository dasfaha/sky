#ifndef _qip_ast_alloca_h
#define _qip_ast_alloca_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a alloca() function in the AST.
typedef struct {
    qip_ast_node *expr;
    qip_ast_node *return_type_ref;
} qip_ast_alloca;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_alloca_create(qip_ast_node *expr);

void qip_ast_alloca_free(qip_ast_node *node);

int qip_ast_alloca_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_alloca_codegen(qip_ast_node *node, qip_module *module,
	LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_alloca_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_alloca_get_type(qip_ast_node *node, qip_ast_node **type);

//--------------------------------------
// Type refs
//--------------------------------------

int qip_ast_alloca_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_alloca_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_alloca_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_alloca_dump(qip_ast_node *node, bstring ret);

#endif
