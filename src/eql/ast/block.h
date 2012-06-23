#ifndef _eql_ast_block_h
#define _eql_ast_block_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

typedef struct eql_ast_block eql_ast_block;

// Represents a block in the AST.
struct eql_ast_block {
    bstring name;
    struct eql_ast_node **exprs;
    unsigned int expr_count;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_block_create(bstring name, eql_ast_node **exprs, unsigned int expr_count,
    eql_ast_node **ret);

void eql_ast_block_free(eql_ast_node *node);


//--------------------------------------
// Expression Management
//--------------------------------------

int eql_ast_block_add_expr(eql_ast_node *block, eql_ast_node *expr);

int eql_ast_block_add_exprs(eql_ast_node *block,
    eql_ast_node **exprs, unsigned int expr_count);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_block_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Types
//--------------------------------------

int eql_ast_block_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_block_dump(eql_ast_node *node, bstring ret);

#endif