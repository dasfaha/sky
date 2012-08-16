#ifndef _eql_ast_binary_expr_h
#define _eql_ast_binary_expr_h


//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of binary expressions.
typedef enum {
    EQL_BINOP_PLUS,
    EQL_BINOP_MINUS,
    EQL_BINOP_MUL,
    EQL_BINOP_DIV,
    EQL_BINOP_EQUALS,
} eql_ast_binop_e;

// Represents a binary expression in the AST.
typedef struct {
    eql_ast_binop_e operator;
    eql_ast_node *lhs;
    eql_ast_node *rhs;
    eql_ast_node *boolean_type_ref;
} eql_ast_binary_expr;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_binary_expr_create(eql_ast_binop_e operator,
    eql_ast_node *lhs, eql_ast_node *rhs);

void eql_ast_binary_expr_free(eql_ast_node *node);

int eql_ast_binary_expr_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_binary_expr_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_binary_expr_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_binary_expr_get_type(eql_ast_node *node, eql_module *module,
    eql_ast_node **type_ref);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_binary_expr_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_binary_expr_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_binary_expr_dump(eql_ast_node *node, bstring ret);

#endif