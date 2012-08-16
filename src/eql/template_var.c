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
eql_ast_node *eql_ast_template_var_create(bstring name)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_TEMPLATE_VAR;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;
    node->template_var.name = bstrcpy(name);
    check_mem(node->template_var.name);
    return node;

error:
    eql_ast_node_free(node);
    return NULL;
}

// Frees a template variable AST node from memory.
//
// node - The AST node to free.
void eql_ast_template_var_free(eql_ast_node *node)
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
int eql_ast_template_var_copy(eql_ast_node *node, eql_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    eql_ast_node *clone = eql_ast_template_var_create(node->template_var.name);
    check_mem(clone);

    *ret = clone;
    return 0;

error:
    eql_ast_node_free(clone);
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
int eql_ast_template_var_dump(eql_ast_node *node, bstring ret)
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
