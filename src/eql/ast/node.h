#ifndef _eql_ast_node_h
#define _eql_ast_node_h

#include "access.h"
#include "int_literal.h"
#include "float_literal.h"
#include "binary_expr.h"
#include "var_ref.h"
#include "var_decl.h"
#include "farg.h"
#include "fproto.h"
#include "function.h"
#include "fcall.h"
#include "block.h"
#include "method.h"
#include "property.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Defines the types of expressions available.
typedef enum {
    EQL_AST_TYPE_INT_LITERAL,
    EQL_AST_TYPE_FLOAT_LITERAL,
    EQL_AST_TYPE_BINARY_EXPR,
    EQL_AST_TYPE_VAR_REF,
    EQL_AST_TYPE_VAR_DECL,
    EQL_AST_TYPE_FARG,
    EQL_AST_TYPE_FPROTO,
    EQL_AST_TYPE_FUNCTION,
    EQL_AST_TYPE_FCALL,
    EQL_AST_TYPE_BLOCK,
    EQL_AST_TYPE_METHOD,
    EQL_AST_TYPE_PROPERTY
} eql_ast_node_type_e;

// Represents an node in the AST.
typedef struct eql_ast_node {
    eql_ast_node_type_e type;
    union {
        eql_ast_int_literal int_literal;
        eql_ast_float_literal float_literal;
        eql_ast_binary_expr binary_expr;
        eql_ast_var_ref var_ref;
        eql_ast_var_decl var_decl;
        eql_ast_farg farg;
        eql_ast_fproto fproto;
        eql_ast_function function;
        eql_ast_fcall fcall;
        eql_ast_block block;
        eql_ast_method method;
        eql_ast_property property;
    };
} eql_ast_node;


//==============================================================================
//
// Functions
//
//==============================================================================

void eql_ast_node_free(eql_ast_node *node);

#endif