#ifndef _qip_ast_var_ref_h
#define _qip_ast_var_ref_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable reference in the AST.
typedef struct {
    bstring name;
} qip_ast_var_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_var_ref_create(bstring name);

void qip_ast_var_ref_free(qip_ast_node *node);

int qip_ast_var_ref_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_var_ref_codegen(qip_ast_node *node, qip_module *module,
	LLVMValueRef *value);

int qip_ast_var_ref_get_pointer(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_var_ref_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_var_ref_get_type(qip_ast_node *node, qip_ast_node **type);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_var_ref_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_var_ref_dump(qip_ast_node *node, bstring ret);

#endif
