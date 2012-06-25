#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a metadata items.
//
// key   - The name of the metadata item.
// value - The value of the metadata item.
// ret   - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_metadata_item_create(bstring key, bstring value,
                                 struct eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_METADATA_ITEM;
    node->parent = NULL;
    node->metadata_item.key = bstrcpy(key);
    check_mem(node->metadata_item.key);
    node->metadata_item.value = bstrcpy(value);
    check_mem(node->metadata_item.value);
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a variable declaration AST node from memory.
//
// node - The AST node to free.
void eql_ast_metadata_item_free(struct eql_ast_node *node)
{
    if(node->metadata_item.key) bdestroy(node->metadata_item.key);
    node->metadata_item.key = NULL;

    if(node->metadata_item.value) bdestroy(node->metadata_item.value);
    node->metadata_item.value = NULL;
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
int eql_ast_metadata_item_dump(eql_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<metadata-item key='%s' value='%s'>\n", bdatae(node->metadata_item.key, ""), bdatae(node->metadata_item.value, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
