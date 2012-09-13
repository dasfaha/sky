#ifndef _qip_ast_assign_h
#define _qip_ast_assign_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an assignment of an expression to a variable.
typedef struct {
    qip_ast_node *var_ref;
    qip_ast_node *expr;
} qip_ast_var_assign;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_var_assign_create(qip_ast_node *var_ref,
    qip_ast_node *expr);

void qip_ast_var_assign_free(qip_ast_node *node);

int qip_ast_var_assign_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_var_assign_codegen(struct qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_var_assign_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_var_assign_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_var_assign_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_var_assign_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

int qip_ast_var_assign_get_var_refs_by_type(qip_ast_node *node, 
    qip_module *module, bstring type_name, qip_array *array);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_var_assign_dump(qip_ast_node *node, bstring ret);

#endif