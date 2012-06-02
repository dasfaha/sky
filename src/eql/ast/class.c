#include <stdlib.h>
#include "../../dbg.h"

#include "class.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

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
