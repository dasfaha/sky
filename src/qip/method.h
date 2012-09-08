#ifndef _qip_ast_method_h
#define _qip_ast_method_h

#include "bstring.h"
#include "access.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a method in the AST.
typedef struct {
    qip_ast_access_e access;
    qip_ast_node *function;
    qip_ast_node **metadatas;
    unsigned int metadata_count;
} qip_ast_method;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_method_create(qip_ast_access_e access,
    qip_ast_node *function);

void qip_ast_method_free(qip_ast_node *node);

int qip_ast_method_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Metadata Management
//--------------------------------------

int qip_ast_method_add_metadata(qip_ast_node *node,
    qip_ast_node *metadata);

int qip_ast_method_add_metadatas(qip_ast_node *node,
    qip_ast_node **metadatas, unsigned int metadata_count);

int qip_ast_method_get_metadata_node(qip_ast_node *node, bstring name,
    qip_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_method_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_method_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_method_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Misc
//--------------------------------------

int qip_ast_method_generate_this_farg(qip_ast_node *node);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_method_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_method_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_method_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_method_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_method_dump(qip_ast_node *node, bstring ret);

#endif