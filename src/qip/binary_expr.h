#ifndef _qip_ast_binary_expr_h
#define _qip_ast_binary_expr_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of binary expressions.
typedef enum {
    QIP_BINOP_PLUS,
    QIP_BINOP_MINUS,
    QIP_BINOP_MUL,
    QIP_BINOP_DIV,
    QIP_BINOP_EQUALS,
} qip_ast_binop_e;

// Represents a binary expression in the AST.
typedef struct {
    qip_ast_binop_e operator;
    qip_ast_node *lhs;
    qip_ast_node *rhs;
    qip_ast_node *boolean_type_ref;
} qip_ast_binary_expr;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_binary_expr_create(qip_ast_binop_e operator,
    qip_ast_node *lhs, qip_ast_node *rhs);

void qip_ast_binary_expr_free(qip_ast_node *node);

int qip_ast_binary_expr_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_binary_expr_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_binary_expr_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Type
//--------------------------------------

int qip_ast_binary_expr_get_type(qip_ast_node *node, qip_module *module,
    qip_ast_node **type_ref);


//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_binary_expr_validate(qip_ast_node *node, qip_module *module);


//--------------------------------------
// Type refs
//--------------------------------------

int qip_ast_binary_expr_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_binary_expr_dump(qip_ast_node *node, bstring ret);

#endif