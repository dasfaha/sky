#ifndef _eql_ast_assign_h
#define _eql_ast_assign_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an assignment of an expression to a variable.
typedef struct {
    eql_ast_node *var_ref;
    eql_ast_node *expr;
} eql_ast_var_assign;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_var_assign_create(eql_ast_node *var_ref,
    eql_ast_node *expr);

void eql_ast_var_assign_free(eql_ast_node *node);

int eql_ast_var_assign_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_var_assign_codegen(struct eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_var_assign_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_var_assign_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_var_assign_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_var_assign_dump(eql_ast_node *node, bstring ret);

#endif