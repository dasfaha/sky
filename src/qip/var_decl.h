#ifndef _qip_ast_var_decl_h
#define _qip_ast_var_decl_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a variable declaration in the AST.
typedef struct {
    qip_ast_node *type;
    unsigned int subtype_count;
    bstring name;
    qip_ast_node *initial_value;
} qip_ast_var_decl;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_var_decl_create(qip_ast_node *type, bstring name,
    qip_ast_node *initial_value);

void qip_ast_var_decl_free(qip_ast_node *node);

int qip_ast_var_decl_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Hierarchy
//--------------------------------------

int qip_ast_var_decl_replace(qip_ast_node *node, qip_ast_node *old_node,
    qip_ast_node *new_node);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_var_decl_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_var_decl_codegen_destroy(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_var_decl_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_var_decl_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_var_decl_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

int qip_ast_var_decl_get_var_refs_by_type(qip_ast_node *node, qip_module *module,
    bstring type_name, qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_var_decl_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_var_decl_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_var_decl_dump(qip_ast_node *node, bstring ret);

#endif