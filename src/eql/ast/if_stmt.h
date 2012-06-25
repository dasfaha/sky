#ifndef _eql_ast_if_stmt_h
#define _eql_ast_if_stmt_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an "if" statement in the AST.
typedef struct {
    eql_ast_node *condition;
    eql_ast_node *block;
} eql_ast_if_stmt;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_if_stmt_create(eql_ast_node *condition, eql_ast_node *block, 
    eql_ast_node **ret);

void eql_ast_if_stmt_free(struct eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_if_stmt_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_if_stmt_dump(eql_ast_node *node, bstring ret);

#endif