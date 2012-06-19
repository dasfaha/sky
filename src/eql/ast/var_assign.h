#ifndef _eql_ast_assign_h
#define _eql_ast_assign_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an assignment of an expression to a variable.
typedef struct {
    bstring name;
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

int eql_ast_var_assign_create(bstring name, eql_ast_node *expr,
    eql_ast_node **ret);

void eql_ast_var_assign_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_var_assign_codegen(struct eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

#endif