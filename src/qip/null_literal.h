#ifndef _qip_ast_null_literal_h
#define _qip_ast_null_literal_h

#include <inttypes.h>

#include "module.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal null in the AST.
typedef struct {
    qip_ast_node *type_ref;
} qip_ast_null_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_null_literal_create();

void qip_ast_null_literal_free(qip_ast_node *node);

int qip_ast_null_literal_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_null_literal_codegen(struct qip_ast_node *node,
    qip_module *module, LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int qip_ast_null_literal_get_type(qip_ast_node *node,
    qip_ast_node **type_ref);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_null_literal_dump(qip_ast_node *node, bstring ret);

#endif