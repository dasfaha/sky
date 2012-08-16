#ifndef _eql_ast_farg_h
#define _eql_ast_farg_h

#include "bstring.h"

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

eql_ast_node *eql_ast_farg_create(eql_ast_node *var_decl);

void eql_ast_farg_free(eql_ast_node *node);

int eql_ast_farg_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_farg_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_farg_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_farg_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_farg_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_farg_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_farg_dump(eql_ast_node *node, bstring ret);

#endif