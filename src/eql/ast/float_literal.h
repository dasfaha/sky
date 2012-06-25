#ifndef _eql_ast_float_literal_h
#define _eql_ast_float_literal_h

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal floating point number in the AST.
typedef struct {
    double value;
} eql_ast_float_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_float_literal_create(double value, struct eql_ast_node **node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_float_literal_codegen(struct eql_ast_node *node,
    eql_module *module, LLVMValueRef *value);


//--------------------------------------
// Type
//--------------------------------------

int eql_ast_float_literal_get_type(eql_ast_node *node, bstring *type);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_float_literal_dump(eql_ast_node *node, bstring ret);

#endif