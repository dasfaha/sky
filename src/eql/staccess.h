#ifndef _eql_ast_staccess_h
#define _eql_ast_staccess_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of struct access.
typedef enum eql_ast_staccess_type_e {
    EQL_AST_STACCESS_TYPE_PROPERTY,
    EQL_AST_STACCESS_TYPE_METHOD
} eql_ast_staccess_type_e;

// Represents a struct member access in the AST.
typedef struct {
    eql_ast_staccess_type_e type;
    eql_ast_node *var_ref;
    bstring member_name;
    eql_ast_node **args;
    unsigned int arg_count;
} eql_ast_staccess;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_staccess_create(eql_ast_staccess_type_e type,
    eql_ast_node *var_ref, bstring member_name, eql_ast_node **args,
    unsigned int arg_count);

void eql_ast_staccess_free(eql_ast_node *node);

int eql_ast_staccess_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_staccess_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_staccess_get_pointer(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_staccess_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_staccess_get_type(eql_ast_node *node, eql_module *module,
    eql_ast_node **type);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_staccess_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_staccess_dump(eql_ast_node *node, bstring ret);

#endif