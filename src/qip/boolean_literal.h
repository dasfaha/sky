#ifndef _qip_ast_boolean_literal_h
#define _qip_ast_boolean_literal_h

#include <stdbool.h>

#include "module.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal boolean in the AST.
typedef struct {
    bool value;
    qip_ast_node *type_ref;
} qip_ast_boolean_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_boolean_literal_create(bool value);

void qip_ast_boolean_literal_free(qip_ast_node *node);

int qip_ast_boolean_literal_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_boolean_literal_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int qip_ast_boolean_literal_get_type(qip_ast_node *node,
    qip_ast_node **type_ref);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_boolean_literal_dump(qip_ast_node *node, bstring ret);

#endif