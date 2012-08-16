#ifndef _eql_ast_int_literal_h
#define _eql_ast_int_literal_h

#include <inttypes.h>

#include "module.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a literal integer in the AST.
typedef struct {
    int64_t value;
    eql_ast_node *type_ref;
} eql_ast_int_literal;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_int_literal_create(int64_t value);

void eql_ast_int_literal_free(eql_ast_node *node);

int eql_ast_int_literal_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_int_literal_codegen(struct eql_ast_node *node,
    eql_module *module, LLVMValueRef *value);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_int_literal_get_type(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_int_literal_dump(eql_ast_node *node, bstring ret);

#endif