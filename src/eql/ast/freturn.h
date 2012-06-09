#ifndef _eql_ast_freturn_h
#define _eql_ast_freturn_h

#include "../../bstring.h"

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

int eql_ast_freturn_create(eql_ast_node *value, eql_ast_node **node);

void eql_ast_freturn_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_freturn_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *type);

#endif