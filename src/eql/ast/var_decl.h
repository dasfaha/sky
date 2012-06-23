#ifndef _eql_ast_var_decl_h
#define _eql_ast_var_decl_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable declaration in the AST.
typedef struct {
    bstring type;
    bstring name;
} eql_ast_var_decl;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_var_decl_create(bstring type, bstring name, struct eql_ast_node **node);

void eql_ast_var_decl_free(struct eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_var_decl_codegen(struct eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_var_decl_dump(eql_ast_node *node, bstring ret);

#endif