#ifndef _qip_ast_class_h
#define _qip_ast_class_h

#include "bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a class in the AST.
typedef struct {
    bstring name;
    qip_ast_node **template_vars;
    unsigned int template_var_count;
    qip_ast_node **methods;
    unsigned int method_count;
    qip_ast_node **properties;
    unsigned int property_count;
    qip_ast_node **metadatas;
    unsigned int metadata_count;
} qip_ast_class;


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_ast_node *qip_ast_class_create(bstring name, qip_ast_node **methods,
    unsigned int method_count, qip_ast_node **properties,
    unsigned int property_count);

void qip_ast_class_free(qip_ast_node *node);

void qip_ast_class_free_template_vars(qip_ast_node *node);

int qip_ast_class_copy(qip_ast_node *node, qip_ast_node **ret);

//--------------------------------------
// Template Variable Management
//--------------------------------------

int qip_ast_class_add_template_var(qip_ast_node *node,
    qip_ast_node *template_var);

int qip_ast_class_add_template_vars(qip_ast_node *node,
    qip_ast_node **template_vars, unsigned int template_var_count);


//--------------------------------------
// Member Management
//--------------------------------------

int qip_ast_class_add_property(qip_ast_node *class, qip_ast_node *property);

int qip_ast_class_prepend_property(qip_ast_node *class, qip_ast_node *property);

int qip_ast_class_add_method(qip_ast_node *class, qip_ast_node *method);

int qip_ast_class_add_member(qip_ast_node *class, qip_ast_node *member);

int qip_ast_class_add_members(qip_ast_node *class,
    qip_ast_node **members, unsigned int member_count);

int qip_ast_class_get_property_index(qip_ast_node *node,
    bstring property_name, int *property_index);

int qip_ast_class_get_property(qip_ast_node *node,
    bstring property_name, qip_ast_node **property);

int qip_ast_class_get_method(qip_ast_node *node,
    bstring method_name, qip_ast_node **method);


//--------------------------------------
// Metadata Management
//--------------------------------------

int qip_ast_class_add_metadata(qip_ast_node *class, qip_ast_node *metadata);

int qip_ast_class_add_metadatas(qip_ast_node *class,
    qip_ast_node **metadatas, unsigned int metadata_count);

int qip_ast_class_get_metadata_node(qip_ast_node *node, bstring name,
    qip_ast_node **ret);


//--------------------------------------
// Codegen
//--------------------------------------

int qip_ast_class_codegen(qip_ast_node *node, qip_module *module);

int qip_ast_class_codegen_type(qip_module *module, qip_ast_node *node);

int qip_ast_class_codegen_forward_decl(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Preprocessor
//--------------------------------------

int qip_ast_class_preprocess(qip_ast_node *node, qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Find
//--------------------------------------

int qip_ast_class_get_type_refs(qip_ast_node *node,
    qip_ast_node ***type_refs, uint32_t *count);

int qip_ast_class_get_var_refs(qip_ast_node *node, bstring name,
    qip_array *array);

//--------------------------------------
// Dependencies
//--------------------------------------

int qip_ast_class_get_dependencies(qip_ast_node *node,
    bstring **dependencies, uint32_t *count);

//--------------------------------------
// Validation
//--------------------------------------

int qip_ast_class_validate(qip_ast_node *node, qip_module *module);

//--------------------------------------
// Generation
//--------------------------------------

int qip_ast_class_generate_constructor(qip_ast_node *node);

int qip_ast_class_generate_type_ref(qip_ast_node *node, qip_ast_node **ret);

int qip_ast_class_generate_serializer(qip_ast_node *node);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_ast_class_dump(qip_ast_node *node, bstring ret);

#endif