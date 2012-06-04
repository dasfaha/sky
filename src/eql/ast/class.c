#include <stdlib.h>
#include "../../dbg.h"

#include "class.h"
#include "node.h"

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
// ret            - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_create(bstring name,
                         struct eql_ast_node **methods,
                         unsigned int method_count, 
                         struct eql_ast_node **properties,
                         unsigned int property_count, 
                         struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_CLASS;
    node->class.name = bstrcpy(name); check_mem(node->class.name);

    // Copy methods.
    if(method_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * method_count;
        node->class.methods = malloc(sz);
        check_mem(node->class.methods);
        memcpy(node->class.methods, methods, sz);
    }
    else {
        node->class.methods = NULL;
    }
    node->class.method_count = method_count;

    // Copy properties.
    if(property_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * property_count;
        node->class.properties = malloc(sz);
        check_mem(node->class.properties);
        memcpy(node->class.properties, properties, sz);
    }
    else {
        node->class.properties = NULL;
    }
    node->class.property_count = property_count;

    // Clear metadata.
    node->class.metadata_count = 0;
    node->class.metadatas = NULL;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void eql_ast_class_free(struct eql_ast_node *node)
{
    if(node->class.name) bdestroy(node->class.name);
    node->class.name = NULL;
    
    if(node->class.method_count > 0) {
        unsigned int i;
        for(i=0; i<node->class.method_count; i++) {
            eql_ast_node_free(node->class.methods[i]);
            node->class.methods[i] = NULL;
        }
        free(node->class.methods);
        node->class.method_count = 0;
    }

    if(node->class.property_count > 0) {
        unsigned int i;
        for(i=0; i<node->class.property_count; i++) {
            eql_ast_node_free(node->class.properties[i]);
            node->class.properties[i] = NULL;
        }
        free(node->class.properties);
        node->class.property_count = 0;
    }
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
int eql_ast_class_add_property(eql_ast_node *class,
                               eql_ast_node *property)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(property != NULL, "Property is required");
    
    // Append property to class.
    class->class.property_count++;
    class->class.properties = realloc(class->class.properties, sizeof(eql_ast_node*) * class->class.property_count);
    check_mem(class->class.properties);
    class->class.properties[class->class.property_count-1] = property;
    
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
int eql_ast_class_add_method(eql_ast_node *class,
                             eql_ast_node *method)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(method != NULL, "Method is required");
    
    // Append method to class.
    class->class.method_count++;
    class->class.methods = realloc(class->class.methods, sizeof(eql_ast_node*) * class->class.method_count);
    check_mem(class->class.methods);
    class->class.methods[class->class.method_count-1] = method;
    
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
int eql_ast_class_add_member(eql_ast_node *class,
                             eql_ast_node *member)
{
    int rc;
    
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(member != NULL, "Member is required");

    // Add member via the appropriate function.
    if(member->type == EQL_AST_TYPE_PROPERTY) {
        rc = eql_ast_class_add_property(class, member);
        check(rc == 0, "Unable to add property to class");
    }
    else if(member->type == EQL_AST_TYPE_METHOD) {
        rc = eql_ast_class_add_method(class, member);
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
int eql_ast_class_add_members(eql_ast_node *class,
                              eql_ast_node **members,
                              unsigned int member_count)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(members != NULL || member_count == 0, "Members are required");

    // Add each member.
    unsigned int i;
    for(i=0; i<member_count; i++) {
        int rc = eql_ast_class_add_member(class, members[i]);
        check(rc == 0, "Unable to add member to class");
    }
    
    return 0;

error:
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
int eql_ast_class_add_metadata(struct eql_ast_node *class,
                               struct eql_ast_node *metadata)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metadata != NULL, "Metadata is required");
    
    // Append metadata to class.
    class->class.metadata_count++;
    class->class.metadatas = realloc(class->class.metadatas, sizeof(eql_ast_node*) * class->class.metadata_count);
    check_mem(class->class.metadatas);
    class->class.metadatas[class->class.metadata_count-1] = metadata;
    
    return 0;

error:
    return -1;
}

// Adds a list of metatdata tags to a class.
//
// class           - The class to add the metatdata to.
// metatdatas      - A list of metatdatas to add.
// metatdata_count - The number of metatdatas to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_metadatas(eql_ast_node *class,
                                eql_ast_node **metatdatas,
                                unsigned int metatdata_count)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metatdatas != NULL || metatdata_count == 0, "Metadata tags are required");

    // Add each metatdata.
    unsigned int i;
    for(i=0; i<metatdata_count; i++) {
        int rc = eql_ast_class_add_metadata(class, metatdatas[i]);
        check(rc == 0, "Unable to add metatdata to class");
    }
    
    return 0;

error:
    return -1;
}

