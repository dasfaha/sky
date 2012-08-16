#ifndef _eql_ast_var_ref_h
#define _eql_ast_var_ref_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable reference in the AST.
typedef struct {
    bstring name;
} eql_ast_var_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_var_ref_create(bstring name);

void eql_ast_var_ref_free(eql_ast_node *node);

int eql_ast_var_ref_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_var_ref_codegen(eql_ast_node *node, eql_module *module,
	LLVMValueRef *value);

int eql_ast_var_ref_get_pointer(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_var_ref_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_var_ref_get_type(eql_ast_node *node, eql_ast_node **type);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_var_ref_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_var_ref_dump(eql_ast_node *node, bstring ret);

#endif
