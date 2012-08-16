#ifndef _eql_ast_function_h
#define _eql_ast_function_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a function in the AST.
typedef struct {
    bstring name;
    eql_ast_node *return_type;
    eql_ast_node **return_subtypes;
    unsigned int return_subtype_count;
    eql_ast_node **args;
    unsigned int arg_count;
    eql_ast_node *body;
} eql_ast_function;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_function_create(bstring name, eql_ast_node *return_type,
    eql_ast_node **args, unsigned int arg_count,
    eql_ast_node *body);

void eql_ast_function_free(eql_ast_node *node);

int eql_ast_function_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Argument Management
//--------------------------------------

int eql_ast_function_add_arg(eql_ast_node *node, eql_ast_node *farg);

//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_function_codegen(eql_ast_node *node, eql_module *module,
    LLVMValueRef *value);

int eql_ast_function_codegen_prototype_with_name(eql_ast_node *node,
    eql_module *module, bstring function_name, LLVMValueRef *value);

int eql_ast_function_codegen_args(eql_ast_node *node, eql_module *module);

int eql_ast_function_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_function_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Misc
//--------------------------------------

int eql_ast_function_get_class(eql_ast_node *node, eql_ast_node **class_ast);

int eql_ast_function_generate_return_type(eql_ast_node *node, eql_module *module);

int eql_ast_function_get_var_decl(eql_ast_node *node, bstring name,
    eql_ast_node **var_decl);

int eql_ast_function_get_qualified_name(eql_ast_node *node, bstring *name);

//--------------------------------------
// Metadata
//--------------------------------------

int eql_ast_function_get_external_metadata_node(eql_ast_node *node,
    eql_ast_node **external_metadata_node);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_function_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_function_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_function_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_function_dump(eql_ast_node *node, bstring ret);

#endif