#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "node.h"
#include "util.h"
#include "llvm.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_class_generate_empty_method(qip_ast_node *node, bstring name,
    bool use_existing, qip_ast_node **ret);

int qip_ast_class_preprocess_serializable(qip_ast_node *node,
    qip_module *module);

int qip_ast_class_preprocess_hashable(qip_ast_node *node, qip_module *module);

int qip_ast_class_preprocess_hashable_property(qip_ast_node *node,
    qip_module *module);

int qip_ast_class_preprocess_hashable_set_fields_method(qip_ast_node *node,
    qip_module *module, qip_ast_node *hashable_metadata, bstring property_name);

int qip_ast_class_preprocess_hashable_equals_method(qip_ast_node *node,
    qip_module *module, qip_ast_node *hashable_metadata, bstring property_name);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a class.
//
// name           - The name of the class.
// methods        - An array of methods attached to the class.
// method_count   - The number of methods.
// properties     - An array of properties attached to the class.
// property_count - The number of properties.
//
// Returns a class node.
qip_ast_node *qip_ast_class_create(bstring name,
                         struct qip_ast_node **methods,
                         unsigned int method_count, 
                         struct qip_ast_node **properties,
                         unsigned int property_count)
{
    int rc;
    
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_CLASS;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->class.name = bstrcpy(name);
    if(name != NULL) check_mem(node->class.name);
    node->class.template_vars = NULL;
    node->class.template_var_count = 0;
    node->class.methods = NULL;
    node->class.method_count = 0;
    node->class.properties = NULL;
    node->class.property_count = 0;

    // Add methods.
    rc = qip_ast_class_add_members(node, methods, method_count);
    check(rc == 0, "Unable to add methods to class");

    // Add properties
    rc = qip_ast_class_add_members(node, properties, property_count);
    check(rc == 0, "Unable to add properties to class");

    // Clear metadata.
    node->class.metadata_count = 0;
    node->class.metadatas = NULL;
    
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void qip_ast_class_free(struct qip_ast_node *node)
{
    if(node != NULL) {
        if(node->class.name) bdestroy(node->class.name);
        node->class.name = NULL;

        qip_ast_class_free_template_vars(node);

        if(node->class.method_count > 0) {
            unsigned int i;
            for(i=0; i<node->class.method_count; i++) {
                qip_ast_node_free(node->class.methods[i]);
                node->class.methods[i] = NULL;
            }
            free(node->class.methods);
            node->class.method_count = 0;
        }

        if(node->class.property_count > 0) {
            unsigned int i;
            for(i=0; i<node->class.property_count; i++) {
                qip_ast_node_free(node->class.properties[i]);
                node->class.properties[i] = NULL;
            }
            free(node->class.properties);
            node->class.property_count = 0;
        }

        if(node->class.metadata_count > 0) {
            unsigned int i;
            for(i=0; i<node->class.metadata_count; i++) {
                qip_ast_node_free(node->class.metadatas[i]);
                node->class.metadatas[i] = NULL;
            }
            free(node->class.metadatas);
            node->class.metadata_count = 0;
        }
    }
}

// Frees the template variables on a class.
//
// node - The node.
void qip_ast_class_free_template_vars(qip_ast_node *node)
{
    if(node != NULL) {
        unsigned int i;
        for(i=0; i<node->class.template_var_count; i++) {
            qip_ast_node_free(node->class.template_vars[i]);
            node->class.template_vars[i] = NULL;
        }
        free(node->class.template_vars);
        node->class.template_vars = NULL;
        node->class.template_var_count = 0;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_class_create(node->class.name, NULL, 0, NULL, 0);
    check_mem(clone);

    // Copy template vars.
    clone->class.template_var_count = node->class.template_var_count;
    clone->class.template_vars = calloc(clone->class.template_var_count, sizeof(*clone->class.template_vars));
    check_mem(clone->class.template_vars);
    for(i=0; i<clone->class.template_var_count; i++) {
        rc = qip_ast_node_copy(node->class.template_vars[i], &clone->class.template_vars[i]);
        check(rc == 0, "Unable to copy template var");
        if(clone->class.template_vars[i]) clone->class.template_vars[i]->parent = clone;
    }

    // Copy properties.
    clone->class.property_count = node->class.property_count;
    clone->class.properties = calloc(clone->class.property_count, sizeof(*clone->class.properties));
    check_mem(clone->class.properties);
    for(i=0; i<clone->class.property_count; i++) {
        rc = qip_ast_node_copy(node->class.properties[i], &clone->class.properties[i]);
        check(rc == 0, "Unable to copy property");
        if(clone->class.properties[i]) clone->class.properties[i]->parent = clone;
    }

    // Copy methods.
    clone->class.method_count = node->class.method_count;
    clone->class.methods = calloc(clone->class.method_count, sizeof(*clone->class.methods));
    check_mem(clone->class.methods);
    for(i=0; i<clone->class.method_count; i++) {
        rc = qip_ast_node_copy(node->class.methods[i], &clone->class.methods[i]);
        check(rc == 0, "Unable to copy method");
        if(clone->class.methods[i]) clone->class.methods[i]->parent = clone;
    }

    // Copy metadatas.
    clone->class.metadata_count = node->class.metadata_count;
    clone->class.metadatas = calloc(clone->class.metadata_count, sizeof(*clone->class.metadatas));
    check_mem(clone->class.metadatas);
    for(i=0; i<clone->class.metadata_count; i++) {
        rc = qip_ast_node_copy(node->class.metadatas[i], &clone->class.metadatas[i]);
        check(rc == 0, "Unable to copy metadata");
        if(clone->class.metadatas[i]) clone->class.metadatas[i]->parent = clone;
    }

    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Template Variable Management
//--------------------------------------

// Adds a template variable to a class node.
//
// node         - The node.
// template_var - The template variable to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_template_var(qip_ast_node *node,
                                   qip_ast_node *template_var)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type expected to be 'class'");
    check(template_var != NULL, "Template variable required");
    
    // Append template variable to class.
    node->class.template_var_count++;
    node->class.template_vars = realloc(node->class.template_vars, sizeof(qip_ast_node*) * node->class.template_var_count);
    check_mem(node->class.template_vars);
    node->class.template_vars[node->class.template_var_count-1] = template_var;
    
    // Link template variable to class.
    template_var->parent = node;
    
    return 0;

error:
    return -1;
}

// Adds a list of template variables to a class.
//
// node               - The node.
// template_vars      - A list of template variables to add.
// template_var_count - The number of template variables to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_template_vars(qip_ast_node *node,
                                    qip_ast_node **template_vars,
                                    unsigned int template_var_count)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type expected to be 'class'");
    check(template_vars != NULL || template_var_count == 0, "Template variables are required");

    // Add each template variable.
    unsigned int i;
    for(i=0; i<template_var_count; i++) {
        int rc = qip_ast_class_add_template_var(node, template_vars[i]);
        check(rc == 0, "Unable to add template variable to class");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Member Management
//--------------------------------------

// Adds a property to a class.
//
// class    - The class to add the property to.
// property - The property to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_property(qip_ast_node *class,
                               qip_ast_node *property)
{
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(property != NULL, "Property is required");
    
    // Append property to class.
    class->class.property_count++;
    class->class.properties = realloc(class->class.properties, sizeof(qip_ast_node*) * class->class.property_count);
    check_mem(class->class.properties);
    class->class.properties[class->class.property_count-1] = property;
    
    // Link property to class.
    property->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds a property to the beginning of a class property list.
//
// class    - The class to add the property to.
// property - The property to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_prepend_property(qip_ast_node *class,
                                   qip_ast_node *property)
{
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(property != NULL, "Property is required");
    
    // Reallocate properties array.
    class->class.property_count++;
    class->class.properties = realloc(class->class.properties, sizeof(qip_ast_node*) * class->class.property_count);
    check_mem(class->class.properties);
    
    // Shift everything down.
    unsigned int i;
    for(i=class->class.property_count-1; i>0; i--) {
        class->class.properties[i] = class->class.properties[i-1];
    }
    
    // Add property to the beginning.
    class->class.properties[0] = property;
    property->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds a method to a class.
//
// class  - The class to add the method to.
// method - The method to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_method(qip_ast_node *class,
                             qip_ast_node *method)
{
    int rc;
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(method != NULL, "Method is required");
    
    // Append method to class.
    class->class.method_count++;
    class->class.methods = realloc(class->class.methods, sizeof(qip_ast_node*) * class->class.method_count);
    check_mem(class->class.methods);
    class->class.methods[class->class.method_count-1] = method;
    
    // Link method to class.
    method->parent = class;
    
    // Generate the "this" argument for the method's function.
    rc = qip_ast_method_generate_this_farg(method);
    check(rc == 0, "Unable to generate 'this' argument for method: %s", bdata(method->method.function->function.name));
    
    return 0;

error:
    return -1;
}

// Adds either a method or property to a class.
//
// class  - The class to add the member to.
// member - The method or property to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_member(qip_ast_node *class,
                             qip_ast_node *member)
{
    int rc;
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(member != NULL, "Member is required");

    // Add member via the appropriate function.
    if(member->type == QIP_AST_TYPE_PROPERTY) {
        rc = qip_ast_class_add_property(class, member);
        check(rc == 0, "Unable to add property to class");
    }
    else if(member->type == QIP_AST_TYPE_METHOD) {
        rc = qip_ast_class_add_method(class, member);
        check(rc == 0, "Unable to add method to class");
    }
    else {
        sentinel("Invalid class member type: %d", member->type);
    }
    
    return 0;

error:
    return -1;
}

// Adds a list of methods and properties to a class.
//
// class        - The class to add the member to.
// members      - A list of members to add.
// member_count - The number of members to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_members(qip_ast_node *class,
                              qip_ast_node **members,
                              unsigned int member_count)
{
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(members != NULL || member_count == 0, "Members are required");

    // Add each member.
    unsigned int i;
    for(i=0; i<member_count; i++) {
        int rc = qip_ast_class_add_member(class, members[i]);
        check(rc == 0, "Unable to add member to class");
    }
    
    return 0;

error:
    return -1;
}

// Retrieves the index of the property within the class. This is used to
// access members of the generated struct.
//
// node - The class AST node.
// property_name - The name of the property to find.
// property_index - A pointer to where the index should be returned. If the
//                  property is not found then -1 is returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_property_index(qip_ast_node *node,
                                     bstring property_name,
                                     int *property_index)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(property_name != NULL, "Property name required");
    check(property_index != NULL, "Property index return pointer required");

    unsigned int i;
    
    // Initialize the return value.
    *property_index = -1;
    
    // Loop over properties to find one that matches the name.
    for(i=0; i<node->class.property_count; i++) {
        if(biseq(node->class.properties[i]->property.var_decl->var_decl.name, property_name)) {
            *property_index = (int)i;
        }
    }

    return 0;
    
error:
    *property_index = 0;
    return -1;
}

// Retrieves a reference to a property within the class.
//
// node - The class AST node.
// property_name - The name of the property to find.
// property      - A pointer to where the property should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_property(qip_ast_node *node,
                               bstring property_name,
                               qip_ast_node **property)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(property_name != NULL, "Property name required");
    check(property != NULL, "Property return pointer required");

    // Initialize return value.
    *property = NULL;

    // Retrieve property index.
    int index;
    int rc = qip_ast_class_get_property_index(node, property_name, &index);
    check(rc == 0, "Unable to retrieve property");

    // If found then return the reference to the property.
    if(index >= 0) {
        *property = node->class.properties[index];
    }

    return 0;
    
error:
    *property = NULL;
    return -1;
}

// Retrieves a reference to a method within the class.
//
// node        - The class AST node.
// method_name - The name of the method to find.
// property    - A pointer to where the method should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_method(qip_ast_node *node,
                               bstring method_name,
                               qip_ast_node **method)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(method_name != NULL, "Method name required");
    check(method != NULL, "Method return pointer required");
    
    // Initialize return value.
    *method = NULL;
    
    // Loop over methods to find one that matches the name.
    unsigned int i;
    for(i=0; i<node->class.method_count; i++) {
        if(biseq(node->class.methods[i]->method.function->function.name, method_name)) {
            *method = node->class.methods[i];
            return 0;
        }
    }

    return 0;

error:
    *method = NULL;
    return -1;    
}



//--------------------------------------
// Metadata Management
//--------------------------------------

// Adds a metadata tag to a class.
//
// class    - The class to add the metadata to.
// metadata - The metadata to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_metadata(struct qip_ast_node *class,
                               struct qip_ast_node *metadata)
{
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metadata != NULL, "Metadata is required");
    
    // Append metadata to class.
    class->class.metadata_count++;
    class->class.metadatas = realloc(class->class.metadatas, sizeof(qip_ast_node*) * class->class.metadata_count);
    check_mem(class->class.metadatas);
    class->class.metadatas[class->class.metadata_count-1] = metadata;
    
    // Link metadata to class.
    metadata->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds a list of metatdata tags to a class.
//
// class           - The class to add the metatdata to.
// metatdatas      - A list of metatdatas to add.
// metatdata_count - The number of metadatas to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_add_metadatas(qip_ast_node *class,
                                qip_ast_node **metadatas,
                                unsigned int metadata_count)
{
    check(class != NULL, "Class is required");
    check(class->type == QIP_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metadatas != NULL || metadata_count == 0, "Metadata tags are required");

    // Add each metadata.
    unsigned int i;
    for(i=0; i<metadata_count; i++) {
        int rc = qip_ast_class_add_metadata(class, metadatas[i]);
        check(rc == 0, "Unable to add metadata to class");
    }
    
    return 0;

error:
    return -1;
}

// Retrieves the first metadata node with the given name. If no nodes are
// found then NULL is returned.
//
// node - The class node.
// name - The name of the metadata node to search for.
// ret  - A pointer to where the matching metadata node should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_metadata_node(qip_ast_node *node, bstring name,
                                     qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(name != NULL, "Name required");
    check(ret != NULL, "Return pointer required");

    // Initialize return value.
    *ret = NULL;

    // Loop over metadata and search for node.
    unsigned int i;
    for(i=0; i<node->class.metadata_count; i++) {
        if(biseq(node->class.metadatas[i]->metadata.name, name)) {
            *ret = node->class.metadatas[i];
            break;
        }
    }
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the class AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_codegen(qip_ast_node *node, qip_module *module)
{
    int rc;
    unsigned int i;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(module != NULL, "Module required");

    // Only codegen if this is not a template class.
    if(node->class.template_var_count == 0) {
        // Generate the methods of the class.
        unsigned int method_count = node->class.method_count;
        for(i=0; i<method_count; i++) {
            qip_ast_node *method = node->class.methods[i];
            LLVMValueRef func = NULL;
            rc = qip_ast_node_codegen(method, module, &func);
            check(rc == 0, "Unable to codegen method: %s", bdata(method->method.function->function.name));
        }
    }
    
    return 0;

error:
    return -1;
}

// Generates and registers an LLVM struct for a given AST class. This struct
// can then be referenced within the module as the class' type.
//
// module - The compilation unit that contains the class.
// node   - The class AST node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_codegen_type(qip_module *module, qip_ast_node *node)
{
    int rc;
    unsigned int i;

    check(module != NULL, "Module required");
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");

    // Only codegen if this is not a template class.
    if(node->class.template_var_count == 0) {
        LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

        // Generate struct for class and properties.
        LLVMTypeRef llvm_struct = LLVMStructCreateNamed(context, bdata(node->class.name));

        // Generate the properties of the struct.
        unsigned int property_count = node->class.property_count;
        unsigned int element_count;
    
        // If there are no properties then generate a single property.
        LLVMTypeRef *elements;
        if(property_count == 0) {
            elements = malloc(sizeof(LLVMTypeRef));
            element_count = 1;
            elements[0] = LLVMInt64TypeInContext(context);
        }
        else {
            element_count = property_count;
            elements = malloc(sizeof(LLVMTypeRef) * property_count);
            for(i=0; i<property_count; i++) {
                qip_ast_node *property = node->class.properties[i];
                rc = qip_module_get_type_ref(module, property->property.var_decl->var_decl.type, NULL, &elements[i]);
                check(rc == 0, "Unable to retrieve type: %s", bdata(property->property.var_decl->var_decl.type->type_ref.name));
                check(elements[i] != NULL, "Unable to find class: %s", bdata(property->property.var_decl->var_decl.type->type_ref.name));

                // Wrap complex types as pointers.
                if(qip_llvm_is_complex_type(elements[i])) {
                    elements[i] = LLVMPointerType(elements[i], 0);
                }
            }
        }

        // Set the struct body.
        LLVMStructSetBody(llvm_struct, elements, element_count, false);

        // Add type to the module.
        rc = qip_module_add_type_ref(module, node, llvm_struct);    
        check(rc == 0, "Unable to add type to module");
    }
    
    return 0;
    
error:
    return -1;
}

// Recursively generates forward declarations for functions.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_codegen_forward_decl(qip_ast_node *node,
                                        qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_CLASS, "Node type must be 'class'");
    check(module != NULL, "Module required");

    // Only codegen if this is not a template class.
    if(node->class.template_var_count == 0) {
        // Generate the methods of the class.
        unsigned int i;
        for(i=0; i<node->class.method_count; i++) {
            rc = qip_ast_node_codegen_forward_decl(node->class.methods[i], module);
            check(rc == 0, "Unable to codegen method forward declaration");
        }
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the AST node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess(qip_ast_node *node, qip_module *module,
                             qip_ast_processing_stage_e stage)
{
    int rc;
    uint32_t i;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Method & property generation must occur immediately after load so that
    // dependencies can be calculated.
    if(stage == QIP_AST_PROCESSING_STAGE_LOADING) {
        // Preprocess serializable metatag.
        rc = qip_ast_class_preprocess_serializable(node, module);
        check(rc == 0, "Unable to preprocess serializable meta for class");
    
        // Preprocess hashable metatag.
        rc = qip_ast_class_preprocess_hashable(node, module);
        check(rc == 0, "Unable to preprocess hashable meta for class");
    }
    
    // Preprocess properties.
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_preprocess(node->class.properties[i], module, stage);
        check(rc == 0, "Unable to preprocess class property");
    }

    // preprocess methods.
    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_preprocess(node->class.methods[i], module, stage);
        check(rc == 0, "Unable to preprocess class method");
    }
    
    // Preprocess metadata.
    for(i=0; i<node->class.metadata_count; i++) {
        rc = qip_ast_node_preprocess(node->class.metadatas[i], module, stage);
        check(rc == 0, "Unable to preprocess class metadata");
    }
    
    return 0;

error:
    return -1;
}

// Generates a serialization method on a class if one does not exist and
// the class is marked as "Serializable".
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess_serializable(qip_ast_node *node,
                                          qip_module *module)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    struct tagbstring function_name = bsStatic("serialize");
    struct tagbstring serializer_str = bsStatic("serializer");
    struct tagbstring this_str = bsStatic("this");

    // Find Serializable metatag.
    struct tagbstring metadata_name_str = bsStatic("Serializable");
    qip_ast_node *metadata = NULL;
    rc = qip_ast_class_get_metadata_node(node, &metadata_name_str, &metadata);
    check(rc == 0, "Unable to retrieve Serializable metadata node for class");
    
    // Find existing method.
    qip_ast_node *method = NULL;
    rc = qip_ast_class_get_method(node, &function_name, &method);
    check(rc == 0, "Unable to retrieve serialize method");
    
    // If metatag doesn't exist or the method already exists then exit.
    if(metadata == NULL || method != NULL) {
        return 0;
    }
    
    // Generate empty method.
    rc = qip_ast_class_generate_empty_method(node, &function_name, true, &method);
    check(rc == 0, "Unable to generate constructor method");
    qip_ast_node *function = method->method.function;
    qip_ast_node *block = function->function.body;
    qip_ast_block_free_exprs(block);

    // Add serializer argument.
    qip_ast_node *serializer_type_ref = qip_ast_type_ref_create_cstr("Serializer");
    qip_ast_node *serializer_var_decl = qip_ast_var_decl_create(serializer_type_ref, &serializer_str, NULL);
    qip_ast_node *serializer_farg = qip_ast_farg_create(serializer_var_decl);
    rc = qip_ast_function_add_arg(function, serializer_farg);
    check(rc == 0, "Unable to add serializer argument to the serialize method");

    // Calculate number of serializable fields.
    uint32_t key_count = 0;
    for(i=0; i<node->class.property_count; i++) {
        if(qip_is_serializable_type(node->class.properties[i]->property.var_decl->var_decl.type)) {
            key_count++;
        }
    }

    // Add pack map call.
    struct tagbstring pack_map_name = bsStatic("packMap");
    qip_ast_node *pack_args[2];
    pack_args[0] = qip_ast_int_literal_create(key_count);
    qip_ast_node *method_invoke = qip_ast_var_ref_create_method_invoke(&serializer_str, &pack_map_name, pack_args, 1);
    rc = qip_ast_block_add_expr(block, method_invoke);
    check(rc == 0, "Unable to add expression to block");

    // Add pack calls for each serializable property.
    struct tagbstring pack_int_name = bsStatic("packInt");
    struct tagbstring pack_float_name = bsStatic("packFloat");
    struct tagbstring pack_string_name = bsStatic("packString");
    
    for(i=0; i<node->class.property_count; i++) {
        qip_ast_node *property = node->class.properties[i];
        qip_ast_node *var_decl = property->property.var_decl;

        if(qip_is_serializable_type(var_decl->var_decl.type)) {
            // Determine pack method to use for property.
            bstring pack_method_name = NULL;
            if(biseqcstr(var_decl->var_decl.type->type_ref.name, "Int") == 1) {
                pack_method_name = &pack_int_name;
            }
            else if(biseqcstr(var_decl->var_decl.type->type_ref.name, "Float") == 1) {
                pack_method_name = &pack_float_name;
            }
            else {
                sentinel("Unsupported serialization type");
            }
            
            // Pack key.
            pack_args[0] = qip_ast_string_literal_create(var_decl->var_decl.name);
            method_invoke = qip_ast_var_ref_create_method_invoke(&serializer_str, &pack_string_name, pack_args, 1);
            rc = qip_ast_block_add_expr(block, method_invoke);
            check(rc == 0, "Unable to add key serialization expression");

            // Pack value.
            pack_args[0] = qip_ast_var_ref_create_property_access(&this_str, var_decl->var_decl.name);
            method_invoke = qip_ast_var_ref_create_method_invoke(&serializer_str, pack_method_name, pack_args, 1);
            rc = qip_ast_block_add_expr(block, method_invoke);
            check(rc == 0, "Unable to add value serialization expression");
        }
    }
    
    // Add void return.
    qip_ast_node *freturn = qip_ast_freturn_create(NULL);
    rc = qip_ast_block_add_expr(block, freturn);
    check(rc == 0, "Unable to add serialize return");
    
    return 0;

error:
    return -1;
}

// Generates a hashCode property and support methods when a class is marked
// as [Hashable].
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess_hashable(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Find Hashable metatag.
    struct tagbstring hashable_str = bsStatic("Hashable");
    qip_ast_node *hashable_metadata = NULL;
    rc = qip_ast_class_get_metadata_node(node, &hashable_str, &hashable_metadata);
    check(rc == 0, "Unable to retrieve Hashable metadata node for class");
    
    // If metatag exists then generate property and methods
    if(hashable_metadata != NULL) {
        // Retrieve hashable property name.
        bstring property_name = NULL;
        rc = qip_ast_metadata_get_item_value(hashable_metadata, NULL, &property_name);
        check(rc == 0, "Unable to retrieve hashable property name");
     
        // Validate that the hashable property name exists.
        if(property_name == NULL || blength(property_name) == 0) {
            msg = bformat("Hashable property name cannot be blank for class '%s'", bdata(node->class.name));
        }

        // Generate properties and methods if there are no validation errors.
        if(!msg) {
            // Generate hashCode property.
            rc = qip_ast_class_preprocess_hashable_property(node, module);
            check(rc == 0, "Unable to generate hashCode property");

            // Generate setHashFields() method.
            rc = qip_ast_class_preprocess_hashable_set_fields_method(node, module, hashable_metadata, property_name);
            check(rc == 0, "Unable to generate setHashFields property");

            // Generate equals() method.
            rc = qip_ast_class_preprocess_hashable_equals_method(node, module, hashable_metadata, property_name);
            check(rc == 0, "Unable to generate equals property");
        }
    }

    // If we have an error message then add it.
    if(msg != NULL) {
        rc = qip_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }
    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;
}

// Generates a hashCode property on the class.
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess_hashable_property(qip_ast_node *node,
                                               qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    struct tagbstring int_str = bsStatic("Int");
    struct tagbstring hash_code_str = bsStatic("hashCode");

    // Generate hashCode property.
    qip_ast_node *type_ref = qip_ast_type_ref_create(&int_str);
    check_mem(type_ref);
    qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, &hash_code_str, NULL);
    var_decl->generated = true;
    check_mem(var_decl);
    qip_ast_node *property = qip_ast_property_create(QIP_ACCESS_PRIVATE, var_decl);
    property->generated = true;
    check_mem(property);

    // Add the property to the beginning of the struct. This is done so that
    // we can easily access it in the Map code.
    rc = qip_ast_class_prepend_property(node, property);
    check(rc == 0, "Unable to prepend hashCode property to class");
    
    return 0;

error:
    qip_ast_type_ref_free(type_ref);
    qip_ast_var_decl_free(var_decl);
    qip_ast_property_free(property);
    return -1;
}

// Generates a setHashFields() method on the class.
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess_hashable_set_fields_method(qip_ast_node *node,
                                                       qip_module *module,
                                                       qip_ast_node *hashable_metadata,
                                                       bstring property_name)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(hashable_metadata != NULL, "Hashable metadata required");
    check(property_name != NULL, "Hashable property required");
    
    struct tagbstring function_name = bsStatic("setHashFields");
    struct tagbstring value_type_str = bsStatic("Int");
    struct tagbstring value_str = bsStatic("value");
    struct tagbstring this_str = bsStatic("this");
    struct tagbstring hash_code_str = bsStatic("hashCode");

    qip_ast_node *lhs, *rhs, *var_assign;

    // Generate method skeleton.
    qip_ast_node *method = NULL;
    rc = qip_ast_class_generate_empty_method(node, &function_name, false, &method);
    check(rc == 0, "Unable to generate empty method");
    qip_ast_node *block = method->method.function->function.body;
    
    // Remove function return.
    qip_ast_block_free_exprs(block);
    
    // Add an argument for the hash id.
    qip_ast_node *type_ref = qip_ast_type_ref_create(&value_type_str);
    check_mem(type_ref);
    qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, &value_str, NULL);
    check_mem(var_decl);
    qip_ast_node *farg = qip_ast_farg_create(var_decl); check_mem(farg);
    rc = qip_ast_function_add_arg(method->method.function, farg);
    check(rc == 0, "Unable to add value argument to method");

    // Assign value to id property.
    lhs = qip_ast_var_ref_create_property_access(&this_str, property_name);
    rhs = qip_ast_var_ref_create_value(&value_str);
    var_assign = qip_ast_var_assign_create(lhs, rhs);
    rc = qip_ast_block_add_expr(block, var_assign);
    check(rc == 0, "Unable to prepend variable assignment to block");

    // Assign id property to hashCode.
    lhs = qip_ast_var_ref_create_property_access(&this_str, &hash_code_str);
    rhs = qip_ast_var_ref_create_property_access(&this_str, property_name);
    var_assign = qip_ast_var_assign_create(lhs, rhs);
    rc = qip_ast_block_add_expr(block, var_assign);
    check(rc == 0, "Unable to prepend variable assignment to block");

    // Add void return to block.
    qip_ast_node *freturn = qip_ast_freturn_create(NULL);
    rc = qip_ast_block_add_expr(block, freturn);
    check(rc == 0, "Unable to add void return to method");

    return 0;

error:
    return -1;
}

// Generates a equals() method on the class.
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_preprocess_hashable_equals_method(qip_ast_node *node,
                                                    qip_module *module,
                                                    qip_ast_node *hashable_metadata,
                                                    bstring property_name)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(hashable_metadata != NULL, "Hashable metadata required");
    check(property_name != NULL, "Hashable property required");
    
    struct tagbstring function_name = bsStatic("equals");
    struct tagbstring value_type_str = bsStatic("Int");
    struct tagbstring value_str = bsStatic("value");
    struct tagbstring this_str = bsStatic("this");

    qip_ast_node *lhs, *rhs, *binary_expr;

    // Generate method skeleton.
    qip_ast_node *method = NULL;
    rc = qip_ast_class_generate_empty_method(node, &function_name, false, &method);
    check(rc == 0, "Unable to generate empty method");
    qip_ast_node *block = method->method.function->function.body;

    // Remove function return and return type.
    qip_ast_node_free(method->method.function->function.return_type);
    method->method.function->function.return_type = NULL;
    qip_ast_block_free_exprs(block);

    // Set the return type to boolean.
    struct tagbstring boolean_str = bsStatic("Boolean");
    method->method.function->function.return_type = qip_ast_type_ref_create(&boolean_str);
    check_mem(method->method.function->function.return_type);
    method->method.function->function.return_type->parent = method->method.function;

    // Add an argument for the value.
    qip_ast_node *type_ref = qip_ast_type_ref_create(&value_type_str);
    qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, &value_str, NULL);
    qip_ast_node *farg = qip_ast_farg_create(var_decl);
    rc = qip_ast_function_add_arg(method->method.function, farg);
    check(rc == 0, "Unable to add value argument to method");

    // Compare this.[property_name] to value.
    lhs = qip_ast_var_ref_create_property_access(&this_str, property_name);
    rhs = qip_ast_var_ref_create_value(&value_str);
    binary_expr = qip_ast_binary_expr_create(QIP_BINOP_EQUALS, lhs, rhs);
    qip_ast_node *freturn = qip_ast_freturn_create(binary_expr);
    rc = qip_ast_block_add_expr(block, freturn);
    check(rc == 0, "Unable to add return to method");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Find
//--------------------------------------

// Computes a list of type refs used within this node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_type_refs(qip_ast_node *node,
                                qip_ast_node ***type_refs,
                                uint32_t *count)
{
    int rc;
    uint32_t i;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add property type refs.
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_get_type_refs(node->class.properties[i], type_refs, count);
        check(rc == 0, "Unable to add class property type refs");
    }

    // Add method type refs.
    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_get_type_refs(node->class.methods[i], type_refs, count);
        check(rc == 0, "Unable to add class method type refs");
    }

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_var_refs(qip_ast_node *node, bstring name,
                               qip_array *array)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_get_var_refs(node->class.properties[i], name, array);
        check(rc == 0, "Unable to add class property var refs");
    }

    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_get_var_refs(node->class.methods[i], name, array);
        check(rc == 0, "Unable to add class method var refs");
    }

    return 0;
    
error:
    return -1;
}

// Retrieves all variable reference of a given type name within this node.
//
// node      - The node.
// module    - The module.
// type_name - The type name.
// array     - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_var_refs_by_type(qip_ast_node *node, qip_module *module,
                                       bstring type_name, qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(type_name != NULL, "Type name required");
    check(array != NULL, "Array required");

    uint32_t i;
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_get_var_refs_by_type(node->class.properties[i], module, type_name, array);
        check(rc == 0, "Unable to add class property var refs");
    }

    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_get_var_refs_by_type(node->class.methods[i], module, type_name, array);
        check(rc == 0, "Unable to add class method var refs");
    }

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Dependencies
//--------------------------------------

// Computes a list of class names that this AST node depends on.
//
// node         - The node to compute dependencies for.
// dependencies - A pointer to an array of dependencies.
// count        - A pointer to where the number of dependencies is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_get_dependencies(qip_ast_node *node,
                                   bstring **dependencies,
                                   uint32_t *count)
{
    int rc;
    uint32_t i, j;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Create a temporary array for class dependencies so we can remove
    // template vars later.
    bstring *class_dependencies = NULL;
    uint32_t class_dependency_count = 0;

    // Add property dependencies.
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_get_dependencies(node->class.properties[i], &class_dependencies, &class_dependency_count);
        check(rc == 0, "Unable to add class property dependency");
    }

    // Add method dependencies.
    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_get_dependencies(node->class.methods[i], &class_dependencies, &class_dependency_count);
        check(rc == 0, "Unable to add class method dependencies");
    }

    // Copy over all non-template class dependencies.
    for(i=0; i<class_dependency_count; i++) {
        bool is_template_var = false;
        for(j=0; j<node->class.template_var_count; j++) {
            if(biseq(class_dependencies[i], node->class.template_vars[j]->template_var.name)) {
                is_template_var = true;
                break;
            }
        }
        
        // Only copy to dependencies if this is not a template variable.
        if(!is_template_var) {
            rc = qip_ast_node_add_dependency(class_dependencies[i], dependencies, count);
            check(rc == 0, "Unable to copy class dependency");
        }
    }


    return 0;
    
error:
    qip_ast_node_dependencies_free(&class_dependencies, &class_dependency_count);
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Validates the AST node.
//
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    uint32_t i;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Only validate if this is not a template class.
    if(node->class.template_var_count == 0) {
        // Generate constructor if one does not exist.
        rc = qip_ast_class_generate_constructor(node);
        check(rc == 0, "Unable to generate constructor for class");
    
        // Validate properties.
        for(i=0; i<node->class.property_count; i++) {
            rc = qip_ast_node_validate(node->class.properties[i], module);
            check(rc == 0, "Unable to validate class property");
        }

        // Validate methods.
        for(i=0; i<node->class.method_count; i++) {
            rc = qip_ast_node_validate(node->class.methods[i], module);
            check(rc == 0, "Unable to validate class method");
        }
    
        // Validate metadata.
        for(i=0; i<node->class.metadata_count; i++) {
            rc = qip_ast_node_validate(node->class.metadatas[i], module);
            check(rc == 0, "Unable to validate class metadata");
        }
    }
    
    return 0;

error:
    return -1;
}

//--------------------------------------
// Generation
//--------------------------------------

// Generates an empty method with no arguments and no return type.
//
// node         - The class node.
// name         - The name of the method to generate.
// use_existing - A flag stating if an existing method with the same name
//                should be used if found.
// ret          - A pointer to where the new method should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_generate_empty_method(qip_ast_node *node, bstring name,
                                        bool use_existing, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Name required");
    check(ret != NULL, "Return pointer required");
    
    // String constants.
    struct tagbstring void_str = bsStatic("void");

    // Find existing constructor method.
    qip_ast_node *method = NULL;
    rc = qip_ast_class_get_method(node, name, &method);
    check(rc == 0, "Unable to retrieve method");

    // If a method doesn't exist then build one.
    if(method == NULL) {
        // Generate method, function, block.
        qip_ast_node *block = qip_ast_block_create(NULL, NULL, 0);
        check_mem(block);
        block->generated = true;

        qip_ast_node *return_type_ref = qip_ast_type_ref_create(&void_str);
        check_mem(return_type_ref);
        return_type_ref->generated = true;

        qip_ast_node *function = qip_ast_function_create(name, return_type_ref, NULL, 0, block);
        check_mem(function);
        function->generated = true;

        method = qip_ast_method_create(QIP_ACCESS_PRIVATE, function);
        check_mem(method);
        method->generated = true;

        // Add void return to block.
        qip_ast_node *freturn = qip_ast_freturn_create(NULL);
        rc = qip_ast_block_add_expr(block, freturn);
        check(rc == 0, "Unable to add void return to empty method");

        // Attach the method to the class.
        rc = qip_ast_class_add_method(node, method);
        check(rc == 0, "Unable to add method '%s' to class '%s'", bdata(name), bdata(node->class.name));
    }
    // Throw an error if method is found 
    else if(!use_existing) {
        sentinel("Method '%s' already exists on class '%s'", bdata(name), bdata(node->class.name));
    }
    
    // Return method.
    *ret = method;

    return 0;

error:
    *ret = NULL;
    return -1;
}

// Generates a constructor on a class if one does not already exist.
//
// node   - The class node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_generate_constructor(qip_ast_node *node)
{
    int rc;
    check(node != NULL, "Node required");
    
    struct tagbstring constructor_name = bsStatic("init");
    
    // Find existing constructor method.
    qip_ast_node *method = NULL;
    rc = qip_ast_class_get_method(node, &constructor_name, &method);
    check(rc == 0, "Unable to retrieve constructor method");
    
    // If no existing constructor exists then create one.
    if(method == NULL) {
        rc = qip_ast_class_generate_empty_method(node, &constructor_name, true, &method);
        check(rc == 0, "Unable to generate constructor method");
    }
    
    // Retrieve block from method.
    qip_ast_node *block = method->method.function->function.body;
    check(block != NULL, "Block required");
    
    // Create an array with enough room for null assignments and existing expressions.
    uint32_t expr_count = node->class.property_count + block->block.expr_count;
    qip_ast_node **exprs = malloc(sizeof(qip_ast_node*) * expr_count);
    check_mem(exprs);
    
    // Prepend null assignments.
    unsigned int i;
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_property_generate_initializer(node->class.properties[i], &exprs[i]);
        check(rc == 0, "Unable to generate property initializer");
    }
    
    // Add original block expressions at the end.
    for(i=0; i<block->block.expr_count; i++) {
        unsigned int index = node->class.property_count + i;
        exprs[index] = block->block.exprs[i];
    }
    
    // Clear out old expressions on block.
    if(block->block.exprs) free(block->block.exprs);
    block->block.exprs = NULL;
    block->block.expr_count = 0;
    
    // Add new expressions.
    rc = qip_ast_block_add_exprs(block, exprs, expr_count);
    check(rc == 0, "Unable to add expressions to constructor block");

    return 0;

error:
    if(exprs) free(exprs);
    return -1;
}

// Generates a type reference to this class. If template variables are used
// with this class then they are included.
//
// node - The class node.
// ret  - A pointer to where the new type ref should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_class_generate_type_ref(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");
    
    // Create type ref;
    qip_ast_node *type_ref = qip_ast_type_ref_create(node->class.name);
    check_mem(type_ref);
    
    // Loop over template variables and add subtypes.
    unsigned int i;
    for(i=0; i<node->class.template_var_count; i++) {
        // Create subtype with template variable name.
        qip_ast_node *template_var = node->class.template_vars[i];
        qip_ast_node *subtype_ref = qip_ast_type_ref_create(template_var->template_var.name);
        check_mem(subtype_ref);
        
        // Add subtype to base type ref.
        rc = qip_ast_type_ref_add_subtype(type_ref, subtype_ref);
        check(rc == 0, "Unable to add subtype");
    }
    
    // Return type ref.
    *ret = type_ref;
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Debugging
//--------------------------------------

// Append the contents of the AST node to the string.
// 
// node - The node to dump.
// ret  - A pointer to the bstring to concatenate to.
//
// Return 0 if successful, otherwise returns -1.s
int qip_ast_class_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<class name='%s'>\n", bdatae(node->class.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->class.template_var_count; i++) {
        rc = qip_ast_node_dump(node->class.template_vars[i], ret);
        check(rc == 0, "Unable to dump class template variable");
    }
    for(i=0; i<node->class.method_count; i++) {
        rc = qip_ast_node_dump(node->class.methods[i], ret);
        check(rc == 0, "Unable to dump class method");
    }
    for(i=0; i<node->class.property_count; i++) {
        rc = qip_ast_node_dump(node->class.properties[i], ret);
        check(rc == 0, "Unable to dump class property");
    }
    for(i=0; i<node->class.metadata_count; i++) {
        rc = qip_ast_node_dump(node->class.metadatas[i], ret);
        check(rc == 0, "Unable to dump class metadata");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
