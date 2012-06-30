#ifndef _eql_ast_for_each_stmt_h
#define _eql_ast_for_each_stmt_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an "if" statement in the AST.
typedef struct {
    eql_ast_node *var_decl;
    eql_ast_node *enumerator;
    eql_ast_node *block;
} eql_ast_for_each_stmt;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_for_each_stmt_create(eql_ast_node *var_decl,
    eql_ast_node *enumerator, eql_ast_node *block, eql_ast_node **ret);

void eql_ast_for_each_stmt_free(struct eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_for_each_stmt_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Types
//--------------------------------------

int eql_ast_for_each_stmt_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_for_each_stmt_dump(eql_ast_node *node, bstring ret);

#endif