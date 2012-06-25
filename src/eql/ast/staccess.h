#ifndef _eql_ast_staccess_h
#define _eql_ast_staccess_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a struct member access in the AST.
typedef struct {
    eql_ast_node *var_ref;
    bstring member_name;
} eql_ast_staccess;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_staccess_create(eql_ast_node *var_ref, bstring member_name,
    eql_ast_node **ret);

void eql_ast_staccess_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_staccess_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_staccess_get_pointer(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int eql_ast_staccess_get_type(eql_ast_node *node, eql_module *module,
    bstring *type);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_staccess_dump(eql_ast_node *node, bstring ret);

#endif