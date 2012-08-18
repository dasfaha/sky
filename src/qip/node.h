#ifndef _qip_ast_node_h
#define _qip_ast_node_h

#include <llvm-c/Core.h>


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef enum qip_ast_node_type_e qip_ast_node_type_e;
typedef struct qip_ast_node qip_ast_node;

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
enum qip_ast_node_type_e {
    QIP_AST_TYPE_INT_LITERAL,
    QIP_AST_TYPE_FLOAT_LITERAL,
    QIP_AST_TYPE_BOOLEAN_LITERAL,
    QIP_AST_TYPE_NULL_LITERAL,
    QIP_AST_TYPE_BINARY_EXPR,
    QIP_AST_TYPE_VAR_REF,
    QIP_AST_TYPE_VAR_DECL,
    QIP_AST_TYPE_TYPE_REF,
    QIP_AST_TYPE_VAR_ASSIGN,
    QIP_AST_TYPE_STACCESS,
    QIP_AST_TYPE_FARG,
    QIP_AST_TYPE_FRETURN,
    QIP_AST_TYPE_FUNCTION,
    QIP_AST_TYPE_BLOCK,
    QIP_AST_TYPE_IF_STMT,
    QIP_AST_TYPE_FOR_EACH_STMT,
    QIP_AST_TYPE_METHOD,
    QIP_AST_TYPE_PROPERTY,
    QIP_AST_TYPE_CLASS,
    QIP_AST_TYPE_TEMPLATE_VAR,
    QIP_AST_TYPE_MODULE,
    QIP_AST_TYPE_METADATA,
    QIP_AST_TYPE_METADATA_ITEM,
    QIP_AST_TYPE_SIZEOF
};

// Represents an node in the AST.
struct qip_ast_node {
    qip_ast_node_type_e type;
    qip_ast_node *parent;
    int32_t line_no;
    int32_t char_no;
    bool generated;
    union {
        qip_ast_int_literal int_literal;
        qip_ast_float_literal float_literal;
        qip_ast_boolean_literal boolean_literal;
        qip_ast_null_literal null_literal;
        qip_ast_binary_expr binary_expr;
        qip_ast_var_ref var_ref;
        qip_ast_var_decl var_decl;
        qip_ast_type_ref type_ref;
        qip_ast_var_assign var_assign;
        qip_ast_staccess staccess;
        qip_ast_farg farg;
        qip_ast_freturn freturn;
        qip_ast_function function;
        qip_ast_block block;
        qip_ast_if_stmt if_stmt;
        qip_ast_for_each_stmt for_each_stmt;
        qip_ast_method method;
        qip_ast_property property;
        qip_ast_class class;
        qip_ast_template_var template_var;
        qip_ast_module module;
        qip_ast_metadata metadata;
        qip_ast_metadata_item metadata_item;
        qip_ast_sizeof szof;
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

void qip_ast_node_free(qip_ast_node *node);

int qip_ast_node_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Hierarchy
//--------------------------------------

int qip_ast_node_get_depth(qip_ast_node *node, int32_t *depth);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_node_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_node_codegen(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_node_get_var_pointer(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_node_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_node_preprocess(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Types
//--------------------------------------

int qip_ast_node_get_type(qip_ast_node *node, qip_module *module,
    qip_ast_node **type);

int qip_ast_node_get_type_name(qip_ast_node *node, qip_module *module,
    bstring *type);

int qip_ast_node_get_var_decl(qip_ast_node *node, bstring name,
    qip_ast_node **var_decl);


//--------------------------------------
// Type refs
//--------------------------------------

int qip_ast_node_get_type_refs(qip_ast_node *node, qip_ast_node ***type_refs,
    uint32_t *count);

int qip_ast_node_add_type_ref(qip_ast_node *type_ref, qip_ast_node ***type_refs,
    uint32_t *count);

int qip_ast_node_type_refs_free(qip_ast_node ***type_refs, uint32_t *count);


//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_node_get_dependencies(qip_ast_node *node, bstring **dependencies,
    uint32_t *count);

int qip_ast_node_add_dependency(bstring dependency, bstring **dependencies,
    uint32_t *count);

int qip_ast_node_dependencies_free(bstring **dependencies, uint32_t *count);


//--------------------------------------
// Position
//--------------------------------------

int qip_ast_node_set_pos(qip_ast_node *node, int32_t line_no, int32_t char_no);


//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_node_dump(qip_ast_node *node, bstring ret);

int qip_ast_node_dump_stderr(qip_ast_node *node);

#endif