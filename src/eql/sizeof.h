#ifndef _eql_ast_sizeof_h
#define _eql_ast_sizeof_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a sizeof() function in the AST.
typedef struct {
    eql_ast_node *type_ref;
    eql_ast_node *return_type_ref;
} eql_ast_sizeof;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_sizeof_create(eql_ast_node *type_ref);

void eql_ast_sizeof_free(eql_ast_node *node);

int eql_ast_sizeof_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_sizeof_codegen(eql_ast_node *node, eql_module *module,
	LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_sizeof_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_sizeof_get_type(eql_ast_node *node, eql_ast_node **type);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_sizeof_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_sizeof_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_sizeof_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_sizeof_dump(eql_ast_node *node, bstring ret);

#endif
