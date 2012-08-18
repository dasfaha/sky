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

// Creates an AST node for a template variable.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a template variable node.
qip_ast_node *qip_ast_template_var_create(bstring name)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_TEMPLATE_VAR;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->template_var.name = bstrcpy(name);
    check_mem(node->template_var.name);
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a template variable AST node from memory.
//
// node - The AST node to free.
void qip_ast_template_var_free(qip_ast_node *node)
{
    if(node != NULL) {
        if(node->template_var.name) bdestroy(node->template_var.name);
        node->template_var.name = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_template_var_copy(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_template_var_create(node->template_var.name);
    check_mem(clone);

    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
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
int qip_ast_template_var_dump(qip_ast_node *node, bstring ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bformat("<template-var name='%s'>\n", bdatae(node->template_var.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
