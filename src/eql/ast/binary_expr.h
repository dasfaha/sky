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
} eql_ast_binary_expr;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_binary_expr_create(eql_ast_binop_e operator,
    eql_ast_node *lhs, eql_ast_node *rhs, eql_ast_node **ret);

void eql_ast_binary_expr_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_binary_expr_codegen(struct eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int eql_ast_binary_expr_get_type(eql_ast_node *node, eql_module *module,
    bstring *type);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_binary_expr_dump(eql_ast_node *node, bstring ret);

#endif