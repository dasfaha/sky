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

// Creates an AST node for a variable assignment.
//
// var_ref - The variable to assign the expression to.
// expr    - The expression to assign to the variable.
// ret     - A pointer to where the ast node will be returned.
//
// Returns a variable assignment node.
qip_ast_node *qip_ast_var_assign_create(qip_ast_node *var_ref,
                                        qip_ast_node *expr)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_VAR_ASSIGN;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->var_assign.var_ref = var_ref;
    if(var_ref != NULL) {
        var_ref->parent = node;
    }

    node->var_assign.expr = expr;
    if(expr != NULL) {
        expr->parent = node;
    }

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a binary expression AST node from memory.
//
// node - The AST node to free.
void qip_ast_var_assign_free(qip_ast_node *node)
{
    if(node->var_assign.var_ref) qip_ast_node_free(node->var_assign.var_ref);
    node->var_assign.var_ref = NULL;

    if(node->var_assign.expr) qip_ast_node_free(node->var_assign.expr);
    node->var_assign.expr = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_assign_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_var_assign_create(NULL, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->var_assign.var_ref, &clone->var_assign.var_ref);
    check(rc == 0, "Unable to copy var ref");
    if(clone->var_assign.var_ref) clone->var_assign.var_ref->parent = clone;
    
    rc = qip_ast_node_copy(node->var_assign.expr, &clone->var_assign.expr);
    check(rc == 0, "Unable to copy expression");
    if(clone->var_assign.expr) clone->var_assign.expr->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the variable assignment AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_assign_codegen(qip_ast_node *node,
                                qip_module *module,
                                LLVMValueRef *value)
{
    int rc;
    
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_ASSIGN, "Node type must be 'variable assignment'");
    check(module != NULL, "Module required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Find the variable declaration.
    LLVMValueRef ptr = NULL;
    rc = qip_ast_node_get_var_pointer(node->var_assign.var_ref, module, &ptr);
    check(rc == 0 && ptr != NULL, "Unable to retrieve variable reference pointer");

    // Generate expression.
    LLVMValueRef expr;
    
    // If this is a null assignment then generate a null for the specific type.
    if(node->var_assign.expr->type == QIP_AST_TYPE_NULL_LITERAL) {
        expr = LLVMConstNull(LLVMGetElementType(LLVMTypeOf(ptr)));
    }
    // Otherwise go through the normal codegen.
    else {
        rc = qip_ast_node_codegen(node->var_assign.expr, module, &expr);
        check(rc == 0 && expr != NULL, "Unable to codegen variable assignment expression");
    }
    
    // Create a store instruction.
    *value = LLVMBuildStore(builder, expr, ptr);
    check(*value != NULL, "Unable to generate store instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocess the node.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_assign_preprocess(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Preprocess variable reference.
    rc = qip_ast_node_preprocess(node->var_assign.var_ref, module);
    check(rc == 0, "Unable to preprocess variable assignment reference");
    
    // Preprocess expression.
    rc = qip_ast_node_preprocess(node->var_assign.expr, module);
    check(rc == 0, "Unable to preprocess variable assignment expression");
    
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
int qip_ast_var_assign_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Do not allow assignment to a method invocation.
    if(node->var_assign.var_ref->type == QIP_AST_TYPE_STACCESS && 
       node->var_assign.var_ref->staccess.type == QIP_AST_STACCESS_TYPE_METHOD)
    {
        msg = bformat("Illegal assignment to a method invocation");
    }
    
    // Validate variable reference.
    rc = qip_ast_node_validate(node->var_assign.var_ref, module);
    check(rc == 0, "Unable to validate variable assignment reference");
    
    // Validate expression.
    rc = qip_ast_node_validate(node->var_assign.expr, module);
    check(rc == 0, "Unable to validate variable assignment expression");
    
    // If we have an error message then add it.
    if(msg != NULL) {
        rc = qip_module_add_error(module, node, msg);
        check(rc == 0, "Unable to add module error");
    }

    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;
}


//--------------------------------------
// Type refs
//--------------------------------------

// Computes a list of type references used by a node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_assign_get_type_refs(qip_ast_node *node,
                                      qip_ast_node ***type_refs,
                                      uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    rc = qip_ast_node_get_type_refs(node->var_assign.expr, type_refs, count);
    check(rc == 0, "Unable to add var assign expr");

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
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
int qip_ast_var_assign_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    check(bcatcstr(ret, "<var-assign>\n") == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->var_assign.var_ref != NULL) {
        rc = qip_ast_node_dump(node->var_assign.var_ref, ret);
        check(rc == 0, "Unable to dump variable reference");
    }
    if(node->var_assign.expr != NULL) {
        rc = qip_ast_node_dump(node->var_assign.expr, ret);
        check(rc == 0, "Unable to dump expression");
    }

    return 0;

error:
    return -1;
}
