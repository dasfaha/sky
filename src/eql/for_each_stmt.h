#ifndef _eql_ast_for_each_stmt_h
#define _eql_ast_for_each_stmt_h

#include "bstring.h"

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

eql_ast_node *eql_ast_for_each_stmt_create(eql_ast_node *var_decl,
    eql_ast_node *enumerator, eql_ast_node *block);

void eql_ast_for_each_stmt_free(struct eql_ast_node *node);

int eql_ast_for_each_stmt_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_for_each_stmt_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_for_each_stmt_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Types
//--------------------------------------

int eql_ast_for_each_stmt_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_for_each_stmt_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_for_each_stmt_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_for_each_stmt_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_for_each_stmt_dump(eql_ast_node *node, bstring ret);

#endif