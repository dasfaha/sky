#ifndef _qip_ast_function_h
#define _qip_ast_function_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a function in the AST.
typedef struct {
    bool bound;
    bstring name;
    qip_ast_node *return_type;
    qip_ast_node **return_subtypes;
    unsigned int return_subtype_count;
    qip_ast_node **args;
    unsigned int arg_count;
    qip_ast_node *body;
} qip_ast_function;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_function_create(bstring name, qip_ast_node *return_type,
    qip_ast_node **args, unsigned int arg_count,
    qip_ast_node *body);

void qip_ast_function_free(qip_ast_node *node);

int qip_ast_function_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Argument Management
//--------------------------------------

int qip_ast_function_add_arg(qip_ast_node *node, qip_ast_node *farg);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_function_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_function_codegen_prototype_with_name(qip_ast_node *node,
    qip_module *module, bstring function_name, LLVMValueRef *value);

int qip_ast_function_codegen_args(qip_ast_node *node, qip_module *module);

int qip_ast_function_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_function_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Misc
//--------------------------------------

int qip_ast_function_get_class(qip_ast_node *node, qip_ast_node **class_ast);

int qip_ast_function_generate_return_type(qip_ast_node *node, qip_module *module);

int qip_ast_function_get_var_decl(qip_ast_node *node, bstring name,
    qip_ast_node **var_decl);

int qip_ast_function_get_qualified_name(qip_ast_node *node, bstring *name);

//--------------------------------------
// Metadata
//--------------------------------------

int qip_ast_function_get_external_metadata_node(qip_ast_node *node,
    qip_ast_node **external_metadata_node);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_function_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_function_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_function_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_function_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_function_dump(qip_ast_node *node, bstring ret);

#endif