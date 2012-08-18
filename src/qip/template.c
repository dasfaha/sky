#include <stdlib.h>
#include "dbg.h"

#include "template.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a template.
//
// Returns a template.
qip_ast_template *qip_ast_template_create(qip_ast_node *class,
                                          qip_ast_node *type_ref)
{
    qip_ast_template *template = malloc(sizeof(qip_ast_template));
    check_mem(template);
    template->class = class;
    template->type_ref = type_ref;
    return template;

error:
    qip_ast_template_free(template);
    return NULL;
}

// Frees a template.
//
// template - The template.
void qip_ast_template_free(qip_ast_template *template)
{
    if(template != NULL) {
        template->class = NULL;
        template->type_ref = NULL;
        free(template);
    }
}


//--------------------------------------
// Templating
//--------------------------------------

// Applies the template to a given type reference by checking if the template
// name equals the name of one of the class' template variables. If so, it is
// replaced by an instance of the appropriate type reference for this template.
// 
// template - The template to apply.
// 
// Returns 0 if successful, otherwise returns -1.
int qip_ast_template_apply(qip_ast_template *template,
                           qip_ast_node *node)
{
    int rc;
    unsigned int i, j;
    check(template != NULL, "Template required");
    check(node != NULL, "Type reference required");
    check(template->class != NULL, "Template class required");
    check(template->type_ref != NULL, "Template type ref required");
    check(biseq(template->class->class.name, template->type_ref->type_ref.name), "Template class name and type ref name must match");

    // Extract template variables & subtypes.
    qip_ast_node **template_vars = template->class->class.template_vars;
    unsigned int template_var_count = template->class->class.template_var_count;
    qip_ast_node **subtypes = template->type_ref->type_ref.subtypes;
    unsigned int subtype_count = template->type_ref->type_ref.subtype_count;
    check(template_var_count == subtype_count, "Template variable count and subtype count must match");
    
    // Retrieve list of type refs within node.
    qip_ast_node **type_refs = NULL;
    unsigned int type_ref_count = 0;
    rc = qip_ast_node_get_type_refs(node, &type_refs, &type_ref_count);
    check(rc == 0, "Unable to retrieve type references from node");

    // Loop over all type references and replace instances of template
    // variables with the appropriate types.
    for(i=0; i<type_ref_count; i++) {
        qip_ast_node *type_ref = type_refs[i];
        
        for(j=0; j<template_var_count; j++) {
            qip_ast_node *template_var = template_vars[j];
            qip_ast_node *subtype = subtypes[j];
            
            // If the type ref name matches the template var name then swap
            // it out for a copy of the matching subtype. For example, if 
            // the type ref name is "T" and the second template var is "T",
            // then copy the second subtype which might be "Event". So
            // a Map<T> would turn into Map<Event>.
            if(biseq(type_ref->type_ref.name, template_var->template_var.name)) {
                // Swap out type name.
                bdestroy(type_ref->type_ref.name);
                type_ref->type_ref.name = bstrcpy(subtype->type_ref.name);
                check_mem(type_ref->type_ref.name);
            }
        }
    }
    
    return 0;

error:
    return -1;
}

