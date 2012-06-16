#ifndef _eql_ast_class_h
#define _eql_ast_class_h

#include "../../bstring.h"

//==============================================================================
//
// Definitions
//
//==============================================================================

// Represents a class in the AST.
typedef struct {
    bstring name;
    struct eql_ast_node **methods;
    unsigned int method_count;
    struct eql_ast_node **properties;
    unsigned int property_count;
    struct eql_ast_node **metadatas;
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

int eql_ast_class_create(bstring name, struct eql_ast_node **methods,
    unsigned int method_count, struct eql_ast_node **properties,
    unsigned int property_count, struct eql_ast_node **ret);

void eql_ast_class_free(struct eql_ast_node *node);


//--------------------------------------
// Member Management
//--------------------------------------

int eql_ast_class_add_property(struct eql_ast_node *class, struct eql_ast_node *property);

int eql_ast_class_add_method(struct eql_ast_node *class, struct eql_ast_node *method);

int eql_ast_class_add_member(struct eql_ast_node *class, struct eql_ast_node *member);

int eql_ast_class_add_members(struct eql_ast_node *class,
    struct eql_ast_node **members, unsigned int member_count);


//--------------------------------------
// Metadata Management
//--------------------------------------

int eql_ast_class_add_metadata(struct eql_ast_node *class, struct eql_ast_node *metadata);

int eql_ast_class_add_metadatas(struct eql_ast_node *class,
    struct eql_ast_node **metadatas, unsigned int metadata_count);


//--------------------------------------
// Codegen
//--------------------------------------

int eql_ast_class_codegen(eql_ast_node *node, eql_module *module);

int eql_ast_class_codegen_type(eql_module *module, eql_ast_node *node);

#endif