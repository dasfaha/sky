#ifndef _qip_ast_farg_h
#define _qip_ast_farg_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable declaration in the AST.
typedef struct qip_ast_farg {
    qip_ast_node *var_decl;
} qip_ast_farg;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_farg_create(qip_ast_node *var_decl);

void qip_ast_farg_free(qip_ast_node *node);

int qip_ast_farg_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_farg_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_farg_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int qip_ast_farg_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_farg_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_farg_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_farg_dump(qip_ast_node *node, bstring ret);

#endif