#ifndef _eql_ast_var_decl_h
#define _eql_ast_var_decl_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable declaration in the AST.
typedef struct {
    eql_ast_node *type;
    unsigned int subtype_count;
    bstring name;
    eql_ast_node *initial_value;
} eql_ast_var_decl;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_var_decl_create(eql_ast_node *type, bstring name,
    eql_ast_node *initial_value);

void eql_ast_var_decl_free(eql_ast_node *node);

int eql_ast_var_decl_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_var_decl_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_var_decl_codegen_destroy(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_var_decl_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_var_decl_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_var_decl_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_var_decl_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_var_decl_dump(eql_ast_node *node, bstring ret);

#endif