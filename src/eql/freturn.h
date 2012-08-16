#ifndef _eql_ast_freturn_h
#define _eql_ast_freturn_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a function return in the AST.
typedef struct eql_ast_freturn {
    eql_ast_node *value;
} eql_ast_freturn;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_freturn_create(eql_ast_node *value);

void eql_ast_freturn_free(eql_ast_node *node);

int eql_ast_freturn_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_freturn_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *type);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_freturn_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_freturn_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_freturn_dump(eql_ast_node *node, bstring ret);

#endif