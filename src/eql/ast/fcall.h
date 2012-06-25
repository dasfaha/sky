#ifndef _eql_ast_fcall_h
#define _eql_ast_fcall_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a function call in the AST.
typedef struct {
    bstring name;
    eql_ast_node **args;
    unsigned int arg_count;
} eql_ast_fcall;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int eql_ast_fcall_create(bstring name, eql_ast_node **args,
    unsigned int arg_count, eql_ast_node **ret);

void eql_ast_fcall_free(eql_ast_node *node);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_fcall_codegen(eql_ast_node *node, eql_module *module,
	LLVMValueRef *value);

//--------------------------------------
// Type
//--------------------------------------

int eql_ast_fcall_get_type(eql_ast_node *node, eql_module *module,
    bstring *type);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_fcall_dump(eql_ast_node *node, bstring ret);

#endif