#ifndef _eql_ast_if_stmt_h
#define _eql_ast_if_stmt_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents an "if" statement in the AST.
typedef struct {
    eql_ast_node **conditions;
    eql_ast_node **blocks;
    eql_ast_node *else_block;
    unsigned int block_count;
} eql_ast_if_stmt;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_if_stmt_create();

void eql_ast_if_stmt_free(struct eql_ast_node *node);

int eql_ast_if_stmt_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Block Management
//--------------------------------------

int eql_ast_if_stmt_add_block(eql_ast_node *node, eql_ast_node *condition,
    eql_ast_node *block);

int eql_ast_if_stmt_add_blocks(eql_ast_node *node, eql_ast_node **conditions,
    eql_ast_node **blocks, unsigned int block_count);

int eql_ast_if_stmt_set_else_block(eql_ast_node *node, eql_ast_node *block);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_if_stmt_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_if_stmt_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_if_stmt_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_if_stmt_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_if_stmt_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_if_stmt_dump(eql_ast_node *node, bstring ret);

#endif