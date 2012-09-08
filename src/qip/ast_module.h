#ifndef _qip_ast_module_h
#define _qip_ast_module_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a module in the AST.
typedef struct {
    bstring name;
    qip_ast_node **classes;
    unsigned int class_count;
    qip_ast_node *main_function;
} qip_ast_module;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_module_create(bstring name, qip_ast_node **classes,
    unsigned int class_count, qip_ast_node *main_function);

void qip_ast_module_free(qip_ast_node *node);

int qip_ast_module_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_module_codegen(qip_ast_node *node, qip_module *module);

int qip_ast_module_codegen_type(qip_module *module, qip_ast_node *node);

int qip_ast_module_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_module_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Class Management
//--------------------------------------

int qip_ast_module_add_class(qip_ast_node *module, qip_ast_node *class);

int qip_ast_module_get_class(qip_ast_node *node, bstring name,
    qip_ast_node **ret);

int qip_ast_module_get_template_class(qip_ast_node *node,
    qip_ast_node *type_ref, qip_ast_node **ret);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_module_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_module_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_module_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_module_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_module_dump(qip_ast_node *node, bstring ret);

#endif