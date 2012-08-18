#include <stdlib.h>
#include "dbg.h"

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
qip_ast_node *qip_ast_metadata_item_create(bstring key, bstring value)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_METADATA_ITEM;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->metadata_item.key = bstrcpy(key);
    if(key != NULL) check_mem(node->metadata_item.key);
    node->metadata_item.value = bstrcpy(value);
    if(value != NULL) check_mem(node->metadata_item.value);
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a variable declaration AST node from memory.
//
// node - The AST node to free.
void qip_ast_metadata_item_free(struct qip_ast_node *node)
{
    if(node->metadata_item.key) bdestroy(node->metadata_item.key);
    node->metadata_item.key = NULL;

    if(node->metadata_item.value) bdestroy(node->metadata_item.value);
    node->metadata_item.value = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_metadata_item_copy(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_metadata_item_create(node->metadata_item.key, node->metadata_item.value);
    check_mem(clone);
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
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
int qip_ast_metadata_item_preprocess(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
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
int qip_ast_metadata_item_validate(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
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
int qip_ast_metadata_item_dump(qip_ast_node *node, bstring ret)
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
