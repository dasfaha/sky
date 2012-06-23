#ifndef _eql_ast_method_h
#define _eql_ast_method_h

#include "../../bstring.h"
#include "access.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Forward declarations.
struct eql_ast_node;

// Represents a method in the AST.
typedef struct {
    eql_ast_access_e access;
    struct eql_ast_node *function;
} eql_ast_method;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_method_create(eql_ast_access_e access,
    struct eql_ast_node *function, struct eql_ast_node **ret);

void eql_ast_method_free(struct eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_method_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);


//--------------------------------------
// Misc
//--------------------------------------

int eql_ast_method_generate_this_farg(eql_ast_node *node);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_method_dump(eql_ast_node *node, bstring ret);

#endif