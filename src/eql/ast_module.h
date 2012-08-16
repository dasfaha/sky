#ifndef _eql_ast_module_h
#define _eql_ast_module_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a module in the AST.
typedef struct {
    bstring name;
    eql_ast_node **classes;
    unsigned int class_count;
    eql_ast_node *main_function;
} eql_ast_module;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_module_create(bstring name, eql_ast_node **classes,
    unsigned int class_count, eql_ast_node *main_function);

void eql_ast_module_free(eql_ast_node *node);

int eql_ast_module_copy(eql_ast_node *node, eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_module_codegen(eql_ast_node *node, eql_module *module);

int eql_ast_module_codegen_type(eql_module *module, eql_ast_node *node);

int eql_ast_module_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_module_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Class Management
//--------------------------------------

int eql_ast_module_add_class(eql_ast_node *module, eql_ast_node *class);

int eql_ast_module_get_class(eql_ast_node *node, bstring name,
    eql_ast_node **ret);

int eql_ast_module_get_template_class(eql_ast_node *node,
    eql_ast_node *type_ref, eql_ast_node **ret);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_module_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_module_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);


//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_module_validate(eql_ast_node *node, eql_module *module);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_module_dump(eql_ast_node *node, bstring ret);

#endif