#ifndef _eql_ast_boolean_literal_h
#define _eql_ast_boolean_literal_h

#include <stdbool.h>

#include "../module.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal boolean in the AST.
typedef struct {
    bool value;
} eql_ast_boolean_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_boolean_literal_create(bool value, eql_ast_node **node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_boolean_literal_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int eql_ast_boolean_literal_get_type(eql_ast_node *node, bstring *type);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_boolean_literal_dump(eql_ast_node *node, bstring ret);

#endif