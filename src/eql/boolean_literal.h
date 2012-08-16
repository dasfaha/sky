#ifndef _eql_ast_boolean_literal_h
#define _eql_ast_boolean_literal_h

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
    eql_ast_node *type_ref;
} eql_ast_boolean_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_boolean_literal_create(bool value);

void eql_ast_boolean_literal_free(eql_ast_node *node);

int eql_ast_boolean_literal_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_boolean_literal_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int eql_ast_boolean_literal_get_type(eql_ast_node *node,
    eql_ast_node **type_ref);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_boolean_literal_dump(eql_ast_node *node, bstring ret);

#endif