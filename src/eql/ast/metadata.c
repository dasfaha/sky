#include <stdlib.h>
#include "../../dbg.h"

#include "metadata.h"
#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a metadata tag.
//
// name       - The name of the metadata tag.
// items      - The key/value pairs attached to the metadata.
// item_count - The number of key/value pairs attached to the metadata.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_create(bstring name,
                            struct eql_ast_node **items,
                            unsigned int item_count,
                            struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METADATA;
    node->parent = NULL;
    node->metadata.name = bstrcpy(name); check_mem(node->metadata.name);

    // Copy items.
    if(item_count > 0) {
        size_t sz = sizeof(eql_ast_node*) * item_count;
        node->metadata.items = malloc(sz);
        check_mem(node->metadata.items);
        
        unsigned int i;
        for(i=0; i<item_count; i++) {
            node->metadata.items[i] = items[i];
            items[i]->parent = node;
        }
    }
    else {
        node->metadata.items = NULL;
    }
    node->metadata.item_count = item_count;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a metadata AST node from memory.
//
// node - The AST node to free.
void eql_ast_metadata_free(struct eql_ast_node *node)
{
    if(node->metadata.name) bdestroy(node->metadata.name);
    node->metadata.name = NULL;

    if(node->metadata.item_count > 0) {
        unsigned int i;
        for(i=0; i<node->metadata.item_count; i++) {
            eql_ast_node_free(node->metadata.items[i]);
            node->metadata.items[i] = NULL;
        }
        free(node->metadata.items);
        node->metadata.item_count = 0;
    }
}
