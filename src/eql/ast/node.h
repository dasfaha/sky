#ifndef _eql_ast_node_h
#define _eql_ast_node_h

#include <llvm-c/Core.h>


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef enum eql_ast_node_type_e eql_ast_node_type_e;
typedef struct eql_ast_node eql_ast_node;

#include "access.h"
#include "int_literal.h"
#include "float_literal.h"
#include "boolean_literal.h"
#include "binary_expr.h"
#include "var_ref.h"
#include "var_decl.h"
#include "var_assign.h"
#include "staccess.h"
#include "farg.h"
#include "freturn.h"
#include "function.h"
#include "fcall.h"
#include "block.h"
#include "if_stmt.h"
#include "method.h"
#include "property.h"
#include "class.h"
#include "module.h"
#include "metadata.h"
#include "metadata_item.h"
#include "../module.h"

// Defines the types of expressions available.
enum eql_ast_node_type_e {
    EQL_AST_TYPE_INT_LITERAL,
    EQL_AST_TYPE_FLOAT_LITERAL,
    EQL_AST_TYPE_BOOLEAN_LITERAL,
    EQL_AST_TYPE_BINARY_EXPR,
    EQL_AST_TYPE_VAR_REF,
    EQL_AST_TYPE_VAR_DECL,
    EQL_AST_TYPE_VAR_ASSIGN,
    EQL_AST_TYPE_STACCESS,
    EQL_AST_TYPE_FARG,
    EQL_AST_TYPE_FRETURN,
    EQL_AST_TYPE_FUNCTION,
    EQL_AST_TYPE_FCALL,
    EQL_AST_TYPE_BLOCK,
    EQL_AST_TYPE_IF_STMT,
    EQL_AST_TYPE_METHOD,
    EQL_AST_TYPE_PROPERTY,
    EQL_AST_TYPE_CLASS,
    EQL_AST_TYPE_MODULE,
    EQL_AST_TYPE_METADATA,
    EQL_AST_TYPE_METADATA_ITEM
};

// Represents an node in the AST.
struct eql_ast_node {
    eql_ast_node_type_e type;
    eql_ast_node *parent;
    union {
        eql_ast_int_literal int_literal;
        eql_ast_float_literal float_literal;
        eql_ast_boolean_literal boolean_literal;
        eql_ast_binary_expr binary_expr;
        eql_ast_var_ref var_ref;
        eql_ast_var_decl var_decl;
        eql_ast_var_assign var_assign;
        eql_ast_staccess staccess;
        eql_ast_farg farg;
        eql_ast_freturn freturn;
        eql_ast_function function;
        eql_ast_fcall fcall;
        eql_ast_block block;
        eql_ast_if_stmt if_stmt;
        eql_ast_method method;
        eql_ast_property property;
        eql_ast_class class;
        eql_ast_module module;
        eql_ast_metadata metadata;
        eql_ast_metadata_item metadata_item;
    };
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

void eql_ast_node_free(eql_ast_node *node);


//--------------------------------------
// Hierarchy
//--------------------------------------

int eql_ast_node_get_depth(eql_ast_node *node, int32_t *depth);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_node_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_node_get_var_pointer(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Types
//--------------------------------------

int eql_ast_node_get_type(eql_ast_node *node, eql_module *module, bstring *type);

int eql_ast_node_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_node_dump(eql_ast_node *node, bstring ret);


#endif