#ifndef _eql_ast_class_h
#define _eql_ast_class_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a class in the AST.
typedef struct {
    bstring name;
    eql_ast_node **template_vars;
    unsigned int template_var_count;
    eql_ast_node **methods;
    unsigned int method_count;
    eql_ast_node **properties;
    unsigned int property_count;
    eql_ast_node **metadatas;
    unsigned int metadata_count;
} eql_ast_class;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

eql_ast_node *eql_ast_class_create(bstring name, eql_ast_node **methods,
    unsigned int method_count, eql_ast_node **properties,
    unsigned int property_count);

void eql_ast_class_free(eql_ast_node *node);

void eql_ast_class_free_template_vars(eql_ast_node *node);

int eql_ast_class_copy(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Template Variable Management
//--------------------------------------

int eql_ast_class_add_template_var(eql_ast_node *node,
    eql_ast_node *template_var);

int eql_ast_class_add_template_vars(eql_ast_node *node,
    eql_ast_node **template_vars, unsigned int template_var_count);


//--------------------------------------
// Member Management
//--------------------------------------

int eql_ast_class_add_property(eql_ast_node *class, eql_ast_node *property);

int eql_ast_class_prepend_property(eql_ast_node *class, eql_ast_node *property);

int eql_ast_class_add_method(eql_ast_node *class, eql_ast_node *method);

int eql_ast_class_add_member(eql_ast_node *class, eql_ast_node *member);

int eql_ast_class_add_members(eql_ast_node *class,
    eql_ast_node **members, unsigned int member_count);

int eql_ast_class_get_property_index(eql_ast_node *node,
    bstring property_name, int *property_index);

int eql_ast_class_get_property(eql_ast_node *node,
    bstring property_name, eql_ast_node **property);

int eql_ast_class_get_method(eql_ast_node *node,
    bstring method_name, eql_ast_node **method);


//--------------------------------------
// Metadata Management
//--------------------------------------

int eql_ast_class_add_metadata(eql_ast_node *class, eql_ast_node *metadata);

int eql_ast_class_add_metadatas(eql_ast_node *class,
    eql_ast_node **metadatas, unsigned int metadata_count);

int eql_ast_class_get_metadata_node(eql_ast_node *node, bstring name,
    eql_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_class_codegen(eql_ast_node *node, eql_module *module);

int eql_ast_class_codegen_type(eql_module *module, eql_ast_node *node);

int eql_ast_class_codegen_forward_decl(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int eql_ast_class_preprocess(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Type refs
//--------------------------------------

int eql_ast_class_get_type_refs(eql_ast_node *node,
    eql_ast_node ***type_refs, uint32_t *count);

//--------------------------------------
// Dependencies
//--------------------------------------

int eql_ast_class_get_dependencies(eql_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int eql_ast_class_validate(eql_ast_node *node, eql_module *module);

//--------------------------------------
// Generation
//--------------------------------------

int eql_ast_class_generate_constructor(eql_ast_node *node);

int eql_ast_class_generate_type_ref(eql_ast_node *node, eql_ast_node **ret);

//--------------------------------------
// Debugging
//--------------------------------------

int eql_ast_class_dump(eql_ast_node *node, bstring ret);

#endif