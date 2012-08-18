#ifndef _qip_ast_freturn_h
#define _qip_ast_freturn_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a function return in the AST.
typedef struct qip_ast_freturn {
    qip_ast_node *value;
} qip_ast_freturn;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_freturn_create(qip_ast_node *value);

void qip_ast_freturn_free(qip_ast_node *node);

int qip_ast_freturn_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_freturn_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *type);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_freturn_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_freturn_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_freturn_dump(qip_ast_node *node, bstring ret);

#endif