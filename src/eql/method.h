#ifndef _eql_ast_method_h
#define _eql_ast_method_h

#include "bstring.h"
#include "access.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a method in the AST.
typedef struct {
    eql_ast_access_e access;
    eql_ast_node *function;
    eql_ast_node **metadatas;
    unsigned int metadata_count;
} eql_ast_method;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_method_create(eql_ast_access_e access,
    eql_ast_node *function);

void eql_ast_method_free(eql_ast_node *node);

int eql_ast_method_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Metadata Management
//--------------------------------------

int eql_ast_method_add_metadata(eql_ast_node *node,
    eql_ast_node *metadata);

int eql_ast_method_add_metadatas(eql_ast_node *node,
    eql_ast_node **metadatas, unsigned int metadata_count);

int eql_ast_method_get_metadata_node(eql_ast_node *node, bstring name,
    eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_method_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_method_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_method_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Misc
//--------------------------------------

int eql_ast_method_generate_this_farg(eql_ast_node *node);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_method_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_method_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_method_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_method_dump(eql_ast_node *node, bstring ret);

#endif