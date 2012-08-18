#ifndef _qip_ast_staccess_h
#define _qip_ast_staccess_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of struct access.
typedef enum qip_ast_staccess_type_e {
    QIP_AST_STACCESS_TYPE_PROPERTY,
    QIP_AST_STACCESS_TYPE_METHOD
} qip_ast_staccess_type_e;

// Represents a struct member access in the AST.
typedef struct {
    qip_ast_staccess_type_e type;
    qip_ast_node *var_ref;
    bstring member_name;
    qip_ast_node **args;
    unsigned int arg_count;
} qip_ast_staccess;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_staccess_create(qip_ast_staccess_type_e type,
    qip_ast_node *var_ref, bstring member_name, qip_ast_node **args,
    unsigned int arg_count);

void qip_ast_staccess_free(qip_ast_node *node);

int qip_ast_staccess_copy(qip_ast_node *node, qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_staccess_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_staccess_get_pointer(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_staccess_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_staccess_get_type(qip_ast_node *node, qip_module *module,
    qip_ast_node **type);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_staccess_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_staccess_dump(qip_ast_node *node, bstring ret);

#endif