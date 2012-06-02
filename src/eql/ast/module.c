#include <stdlib.h>
#include "../../dbg.h"

#include "module.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a module.
//
// name        - The name of the module.
// classes     - An array of classes attached to the module.
// class_count - The number of classes.
// block       - A block of any AST nodes outside class definitions.
// ret         - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_module_create(bstring name,
                         struct eql_ast_node **classes,
                         unsigned int class_count, 
                         struct eql_ast_node *block,
                         struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_MODULE;
    node->module.name = bstrcpy(name); check_mem(node->module.name);
    node->module.block = block;

    // Copy classes.
    if(class_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * class_count;
        node->module.classes = malloc(sz);
        check_mem(node->module.classes);
        memcpy(node->module.classes, classes, sz);
    }
    else {
        node->module.classes = NULL;
    }
    node->module.class_count = class_count;
    
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
void eql_ast_module_free(struct eql_ast_node *node)
{
    if(node->module.name) bdestroy(node->module.name);
    node->module.name = NULL;
    
    if(node->module.block) eql_ast_node_free(node->module.block);
    node->module.block = NULL;
    
    if(node->module.class_count > 0) {
        unsigned int i;
        for(i=0; i<node->module.class_count; i++) {
            eql_ast_node_free(node->module.classes[i]);
            node->module.classes[i] = NULL;
        }
        free(node->module.classes);
        node->module.class_count = 0;
    }
}
