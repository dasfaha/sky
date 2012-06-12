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
// Type
//--------------------------------------

int eql_ast_float_literal_get_type(eql_ast_node *node, bstring *type);

#endif