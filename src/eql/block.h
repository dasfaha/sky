#ifndef _eql_ast_block_h
#define _eql_ast_block_h

#include "bstring.h"

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

eql_ast_node *eql_ast_block_create(bstring name, eql_ast_node **exprs,
    unsigned int expr_count);

void eql_ast_block_free(eql_ast_node *node);

void eql_ast_block_free_exprs(eql_ast_node *node);

int eql_ast_block_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Expression Management
//--------------------------------------

int eql_ast_block_add_expr(eql_ast_node *block, eql_ast_node *expr);

int eql_ast_block_prepend_expr(eql_ast_node *block, eql_ast_node *expr);

int eql_ast_block_add_exprs(eql_ast_node *block,
    eql_ast_node **exprs, unsigned int expr_count);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_block_codegen_block(eql_ast_node *node, eql_module *module,
    LLVMBasicBlockRef *block);

int eql_ast_block_codegen_with_block(eql_ast_node *node, eql_module *module,
    LLVMBasicBlockRef block);

int eql_ast_block_codegen_destroy(eql_ast_node *node, eql_module *module);

int eql_ast_block_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_block_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Types
//--------------------------------------

int eql_ast_block_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);


//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_block_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_block_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_block_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_block_dump(eql_ast_node *node, bstring ret);

#endif