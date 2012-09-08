#include <stdlib.h>
#include "dbg.h"

#include "node.h"
#include "util.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a type reference.
//
// name - The name.
//
// Returns a type ref node.
qip_ast_node *qip_ast_type_ref_create(bstring name)
{
    qip_ast_node *node = calloc(1, sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_TYPE_REF;
    node->type_ref.name = bstrcpy(name);
    if(name != NULL) check_mem(node->type_ref.name);
    
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Creates an AST node for a type reference.
//
// name - The name.
//
// Returns a type ref node.
qip_ast_node *qip_ast_type_ref_create_cstr(char *name)
{
    qip_ast_node *node = NULL;
    bstring name_bstr = bfromcstr(name); check_mem(name_bstr);
    node = qip_ast_type_ref_create(name_bstr);
    bdestroy(name_bstr);
    return node;

error:
    bdestroy(name_bstr);
    qip_ast_node_free(node);
    return NULL;
}

// Frees a type ref node.
//
// node - The node.
void qip_ast_type_ref_free(qip_ast_node *node)
{
    if(node != NULL) {
        if(node->type_ref.name) bdestroy(node->type_ref.name);
        node->type_ref.name = NULL;
        if(node->type_ref.arg_name) bdestroy(node->type_ref.arg_name);
        node->type_ref.arg_name = NULL;

        qip_ast_type_ref_free_subtypes(node);
    }
}

// Frees the return type.
//
// node - The node.
void qip_ast_type_ref_free_return_type(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_node_free(node->type_ref.return_type);
        node->type_ref.return_type = NULL;
    }
}

// Frees the subtypes.
//
// node - The node.
void qip_ast_type_ref_free_subtypes(qip_ast_node *node)
{
    if(node != NULL) {
        unsigned int i;
        for(i=0; i<node->type_ref.subtype_count; i++) {
            qip_ast_node_free(node->type_ref.subtypes[i]);
            node->type_ref.subtypes[i] = NULL;
        }
        free(node->type_ref.subtypes);
        node->type_ref.subtypes = NULL;
        node->type_ref.subtype_count = 0;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_type_ref_create(node->type_ref.name);
    check_mem(clone);

    clone->type_ref.arg_name = bstrcpy(node->type_ref.arg_name);
    if(node->type_ref.arg_name) check_mem(clone->type_ref.arg_name);

    rc = qip_ast_node_copy(node->type_ref.return_type, &clone->type_ref.return_type);
    check(rc == 0, "Unable to copy return type");
    if(clone->type_ref.return_type) clone->type_ref.return_type->parent = clone;

    // Copy subtypes.
    clone->type_ref.subtype_count = node->type_ref.subtype_count;
    clone->type_ref.subtypes = calloc(clone->type_ref.subtype_count, sizeof(*clone->type_ref.subtypes));
    check_mem(clone->type_ref.subtypes);
    for(i=0; i<clone->type_ref.subtype_count; i++) {
        rc = qip_ast_node_copy(node->type_ref.subtypes[i], &clone->type_ref.subtypes[i]);
        check(rc == 0, "Unable to copy subtype");
        if(clone->type_ref.subtypes[i]) clone->type_ref.subtypes[i]->parent = clone;
    }
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Subtype Management
//--------------------------------------

// Adds a subtype.
//
// node     - The node.
// subtype  - The subtype.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_add_subtype(qip_ast_node *node, qip_ast_node *subtype)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    check(subtype != NULL, "Subtype required");
    
    // Append subtype to class.
    node->type_ref.subtype_count++;
    node->type_ref.subtypes = realloc(node->type_ref.subtypes, sizeof(*node->type_ref.subtypes) * node->type_ref.subtype_count);
    check_mem(node->type_ref.subtypes);
    node->type_ref.subtypes[node->type_ref.subtype_count-1] = subtype;
    subtype->parent = node;
    
    return 0;

error:
    return -1;
}

// Adds a list of subtypes.
//
// node          - The node.
// subtypes      - A list of sub types to add.
// subtype_count - The number of sub types to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_add_subtypes(qip_ast_node *node,
                                  qip_ast_node **subtypes,
                                  unsigned int subtype_count)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    check(subtypes != NULL || subtype_count == 0, "Subtypes are required");

    // Add each sub type.
    unsigned int i;
    for(i=0; i<subtype_count; i++) {
        int rc = qip_ast_type_ref_add_subtype(node, subtypes[i]);
        check(rc == 0, "Unable to add subtype to variable declaration");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Return Type Management
//--------------------------------------

// Sets the return type.
//
// node        - The node.
// return_type - The return type.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_set_return_type(qip_ast_node *node, qip_ast_node *return_type)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    check(return_type == NULL || return_type->type == QIP_AST_TYPE_TYPE_REF, "Return type expected to be 'type ref'");
    
    // Assign return type.
    if(node->type_ref.return_type) node->type_ref.return_type->parent = NULL;
    node->type_ref.return_type = return_type;
    if(node->type_ref.return_type) node->type_ref.return_type->parent = node;
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Arg Name
//--------------------------------------

// Sets the argument name. The argument name is used when the type_ref is a
// subtype of a Function type.
//
// node     - The node.
// arg_name - The argument name.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_set_arg_name(qip_ast_node *node, bstring arg_name)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    
    // Assign argument name.
    if(node->type_ref.arg_name) bdestroy(arg_name);
    node->type_ref.arg_name = bstrcpy(arg_name);
    if(arg_name) check_mem(node->type_ref.arg_name);
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Name Resolution
//--------------------------------------

// Calculates the fully qualified name of the type and subtypes.
//
// node - The node.
// ret  - A pointer to where the name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_get_full_name(qip_ast_node *node, bstring *ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    check(ret != NULL, "Return pointer required");

    // Create a comma separated list of subtypes.
    bstring subtype_list = NULL;

    unsigned int i;
    for(i=0; i<node->type_ref.subtype_count; i++) {
        if(subtype_list == NULL) {
            subtype_list = bstrcpy(node->type_ref.subtypes[i]->type_ref.name);
        }
        else {
            bstring tmp = subtype_list;
            subtype_list = bformat("%s,%s", bdata(tmp), bdata(node->type_ref.subtypes[i]->type_ref.name));
            bdestroy(tmp);
        }
        check_mem(subtype_list);
    }
    
    // Append return type to subtype list.
    if(node->type_ref.return_type) {
        bstring tmp = subtype_list;
        subtype_list = bformat("%s:%s", bdatae(subtype_list, ""), bdata(node->type_ref.return_type->type_ref.name));
        bdestroy(tmp);
    }
    
    // Create full name.
    bstring full_name = NULL;
    if(subtype_list == NULL) {
        full_name = bstrcpy(node->type_ref.name);
    }
    else {
        full_name = bformat("%s<%s>", bdata(node->type_ref.name), bdata(subtype_list));
    }
    check_mem(full_name);
    
    // Return full name.
    *ret = full_name;
    
    return 0;

error:
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_preprocess(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type references for a node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    uint32_t i;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add subtypes first.
    for(i=0; i<node->type_ref.subtype_count; i++) {
        rc = qip_ast_node_get_type_refs(node->type_ref.subtypes[i], type_refs, count);
        check(rc == 0, "Unable to add type ref subtype");
    }

    // Add return type.
    if(node->type_ref.return_type != NULL) {
        rc = qip_ast_node_get_type_refs(node->type_ref.return_type, type_refs, count);
        check(rc == 0, "Unable to add type ref return type");
    }

    // Add node to the list.
    rc = qip_ast_node_add_type_ref(node, type_refs, count);
    check(rc == 0, "Unable to add type ref");

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
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
int qip_ast_type_ref_get_dependencies(qip_ast_node *node,
                                      bstring **dependencies,
                                      uint32_t *count)
{
    int rc;
    uint32_t i;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Add type name.
    rc = qip_ast_node_add_dependency(node->type_ref.name, dependencies, count);
    check(rc == 0, "Unable to add type ref dependency");

    // Add return type.
    if(node->type_ref.return_type != NULL) {
        rc = qip_ast_node_get_dependencies(node->type_ref.return_type, dependencies, count);
        check(rc == 0, "Unable to add type ref return type dependency");
    }

    // Add subtypes.
    for(i=0; i<node->type_ref.subtype_count; i++) {
        rc = qip_ast_node_get_dependencies(node->type_ref.subtypes[i], dependencies, count);
        check(rc == 0, "Unable to add type ref subtype dependency");
    }

    return 0;
    
error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Validates the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_validate(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // TODO: Check if type exists.
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Misc
//--------------------------------------

// Determines if a node is a void type.
//
// node - The node.
//
// Returns true if node is void, otherwise returns false.
bool qip_ast_type_ref_is_void(qip_ast_node *node)
{
    return (node != NULL && biseqcstr(node->type_ref.name, "void"));
}

// Flattens a type reference down so that all its subtypes are specified in
// its full name. This occurs during compilation after templates are
// generated.
//
// node - The node.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_type_ref_flatten(qip_ast_node *node)
{
    int rc;
    bstring full_name = NULL;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_TYPE_REF, "Node type expected to be 'type ref'");
    
    // If subtypes are available then clear them and move them into the
    // base type's name. Functions are an exception and should not be
    // flattened.
    if(!qip_is_function_type(node) && node->type_ref.subtype_count > 0) {
        // Generate full name.
        rc = qip_ast_type_ref_get_full_name(node, &full_name);
        check(rc == 0, "Unable to generate type ref name");
        
        // Clear subtypes.
        qip_ast_type_ref_free_subtypes(node);
        
        // Apply new name.
        bdestroy(node->type_ref.name);
        node->type_ref.name = full_name;
        full_name = NULL;
    }
    
    return 0;

error:
    bdestroy(full_name);
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
int qip_ast_type_ref_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<type-ref name='%s' arg-name='%s'>\n", bdatae(node->type_ref.name, ""), bdatae(node->type_ref.arg_name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    unsigned int i;
    for(i=0; i<node->type_ref.subtype_count; i++) {
        rc = qip_ast_node_dump(node->type_ref.subtypes[i], ret);
        check(rc == 0, "Unable to dump subtypes");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
