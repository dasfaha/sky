#ifndef _eql_ast_farg_h
#define _eql_ast_farg_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable declaration in the AST.
typedef struct eql_ast_farg {
    eql_ast_node *var_decl;
} eql_ast_farg;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_farg_create(eql_ast_node *var_decl, eql_ast_node **node);

void eql_ast_farg_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_farg_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_farg_dump(eql_ast_node *node, bstring ret);

#endif