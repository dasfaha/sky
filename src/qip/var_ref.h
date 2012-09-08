#ifndef _qip_ast_var_ref_h
#define _qip_ast_var_ref_h

#include "bstring.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of variable reference.
typedef enum qip_ast_var_ref_type_e {
    QIP_AST_VAR_REF_TYPE_VALUE,
    QIP_AST_VAR_REF_TYPE_INVOKE
} qip_ast_var_ref_type_e;

// Represents a variable reference in the AST.
typedef struct {
    qip_ast_var_ref_type_e type;
    bstring name;
    qip_ast_node *member;
    qip_ast_node **args;
    unsigned int arg_count;
} qip_ast_var_ref;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_var_ref_create(qip_ast_var_ref_type_e type,
    bstring name);

qip_ast_node *qip_ast_var_ref_create_value(bstring name);

qip_ast_node *qip_ast_var_ref_create_invoke(bstring name,
    qip_ast_node **args, unsigned int arg_count);

qip_ast_node *qip_ast_var_ref_create_property_access(bstring name,
    bstring property_name);

qip_ast_node *qip_ast_var_ref_create_method_invoke(bstring name,
    bstring method_name, qip_ast_node **args, unsigned int arg_count);

void qip_ast_var_ref_free(qip_ast_node *node);

int qip_ast_var_ref_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Member Management
//--------------------------------------

int qip_ast_var_ref_set_member(qip_ast_node *node, qip_ast_node *member);

int qip_ast_var_ref_get_last_member(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_var_ref_codegen(qip_ast_node *node, qip_module *module,
    bool gen_ptr, LLVMValueRef *value);

int qip_ast_var_ref_get_pointer(qip_ast_node *node, qip_module *module,
    LLVMValueRef parent_value, LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_var_ref_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_var_ref_get_type(qip_ast_node *node, qip_module *module,
    qip_ast_node **type);

int qip_ast_var_ref_get_class(qip_ast_node *node, qip_module *module,
    qip_ast_node **ret);

int qip_ast_var_ref_get_invoke_function(qip_ast_node *node,
    qip_module *module, qip_ast_node **ret);

int qip_ast_var_ref_get_var_decl(qip_ast_node *node, qip_ast_node **ret);

int qip_ast_var_ref_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_var_ref_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_var_ref_dump(qip_ast_node *node, bstring ret);

#endif
