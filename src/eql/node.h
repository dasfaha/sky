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
#include "null_literal.h"
#include "binary_expr.h"
#include "var_ref.h"
#include "var_decl.h"
#include "type_ref.h"
#include "var_assign.h"
#include "staccess.h"
#include "farg.h"
#include "freturn.h"
#include "function.h"
#include "block.h"
#include "if_stmt.h"
#include "for_each_stmt.h"
#include "method.h"
#include "property.h"
#include "class.h"
#include "template_var.h"
#include "ast_module.h"
#include "metadata.h"
#include "metadata_item.h"
#include "sizeof.h"
#include "template.h"
#include "module.h"

// Defines the types of expressions available.
enum eql_ast_node_type_e {
    EQL_AST_TYPE_INT_LITERAL,
    EQL_AST_TYPE_FLOAT_LITERAL,
    EQL_AST_TYPE_BOOLEAN_LITERAL,
    EQL_AST_TYPE_NULL_LITERAL,
    EQL_AST_TYPE_BINARY_EXPR,
    EQL_AST_TYPE_VAR_REF,
    EQL_AST_TYPE_VAR_DECL,
    EQL_AST_TYPE_TYPE_REF,
    EQL_AST_TYPE_VAR_ASSIGN,
    EQL_AST_TYPE_STACCESS,
    EQL_AST_TYPE_FARG,
    EQL_AST_TYPE_FRETURN,
    EQL_AST_TYPE_FUNCTION,
    EQL_AST_TYPE_BLOCK,
    EQL_AST_TYPE_IF_STMT,
    EQL_AST_TYPE_FOR_EACH_STMT,
    EQL_AST_TYPE_METHOD,
    EQL_AST_TYPE_PROPERTY,
    EQL_AST_TYPE_CLASS,
    EQL_AST_TYPE_TEMPLATE_VAR,
    EQL_AST_TYPE_MODULE,
    EQL_AST_TYPE_METADATA,
    EQL_AST_TYPE_METADATA_ITEM,
    EQL_AST_TYPE_SIZEOF
};

// Represents an node in the AST.
struct eql_ast_node {
    eql_ast_node_type_e type;
    eql_ast_node *parent;
    int32_t line_no;
    int32_t char_no;
    bool generated;
    union {
        eql_ast_int_literal int_literal;
        eql_ast_float_literal float_literal;
        eql_ast_boolean_literal boolean_literal;
        eql_ast_null_literal null_literal;
        eql_ast_binary_expr binary_expr;
        eql_ast_var_ref var_ref;
        eql_ast_var_decl var_decl;
        eql_ast_type_ref type_ref;
        eql_ast_var_assign var_assign;
        eql_ast_staccess staccess;
        eql_ast_farg farg;
        eql_ast_freturn freturn;
        eql_ast_function function;
        eql_ast_block block;
        eql_ast_if_stmt if_stmt;
        eql_ast_for_each_stmt for_each_stmt;
        eql_ast_method method;
        eql_ast_property property;
        eql_ast_class class;
        eql_ast_template_var template_var;
        eql_ast_module module;
        eql_ast_metadata metadata;
        eql_ast_metadata_item metadata_item;
        eql_ast_sizeof szof;
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

int eql_ast_node_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Hierarchy
//--------------------------------------

int eql_ast_node_get_depth(eql_ast_node *node, int32_t *depth);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_node_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_node_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_node_get_var_pointer(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_node_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_node_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Types
//--------------------------------------

int eql_ast_node_get_type(eql_ast_node *node, eql_module *module,
    eql_ast_node **type);

int eql_ast_node_get_type_name(eql_ast_node *node, eql_module *module,
    bstring *type);

int eql_ast_node_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_node_get_type_refs(eql_ast_node *node, eql_ast_node ***type_refs,
    uint32_t *count);

int eql_ast_node_add_type_ref(eql_ast_node *type_ref, eql_ast_node ***type_refs,
    uint32_t *count);

int eql_ast_node_type_refs_free(eql_ast_node ***type_refs, uint32_t *count);


//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_node_get_dependencies(eql_ast_node *node, bstring **dependencies,
    uint32_t *count);

int eql_ast_node_add_dependency(bstring dependency, bstring **dependencies,
    uint32_t *count);

int eql_ast_node_dependencies_free(bstring **dependencies, uint32_t *count);


//--------------------------------------
// Position
//--------------------------------------

int eql_ast_node_set_pos(eql_ast_node *node, int32_t line_no, int32_t char_no);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_node_dump(eql_ast_node *node, bstring ret);

int eql_ast_node_dump_stderr(eql_ast_node *node);

#endif