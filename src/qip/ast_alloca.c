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

// Creates an AST node for a alloca invocation.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_alloca_create(qip_ast_node *expr)
{
    qip_ast_node *node = calloc(1, sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_ALLOCA;
    node->alloca.expr = expr;
    if(expr != NULL) expr->parent = node;

    node->alloca.return_type_ref = qip_ast_type_ref_create_cstr("Ref");
    check_mem(node->alloca.return_type_ref);
    node->alloca.return_type_ref->parent = node;

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void qip_ast_alloca_free(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_node_free(node->alloca.expr);
        node->alloca.expr = NULL;
        
        qip_ast_node_free(node->alloca.return_type_ref);
        node->alloca.return_type_ref = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_alloca_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_alloca_create(NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->alloca.expr, &clone->alloca.expr);
    check(rc == 0, "Unable to copy expression");
    if(clone->alloca.expr) clone->alloca.expr->parent = clone;
    
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

// Recursively generates LLVM code for the AST node.
//
// node    - The node.
// module  - The module.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_alloca_codegen(qip_ast_node *node, qip_module *module,
                           LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_ALLOCA, "Node type expected to be 'alloca'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Generate number of bytes to allocate from expression.
    LLVMValueRef expr_value = NULL;
    rc = qip_ast_node_codegen(node->alloca.expr, module, &expr_value);
    check(rc == 0, "Unable to codegen alloca expression");
    
    // Allocate space for the data.
    *value = LLVMBuildArrayAlloca(builder, LLVMInt8TypeInContext(context), expr_value, "");

    return 0;

error:
    *value = NULL;
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
int qip_ast_alloca_preprocess(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// ret  - A pointer to where the type should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_alloca_get_type(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_ALLOCA, "Node type must be 'alloca'");

    *ret = node->alloca.return_type_ref;
    return 0;

error:
    *ret = NULL;
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
int qip_ast_alloca_get_type_refs(qip_ast_node *node,
                                 qip_ast_node ***type_refs,
                                 uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add expression type.
    rc = qip_ast_node_get_type_refs(node->alloca.expr, type_refs, count);
    check(rc == 0, "Unable to add alloca expression types");

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
int qip_ast_alloca_get_dependencies(qip_ast_node *node,
                                    bstring **dependencies,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    rc = qip_ast_node_get_dependencies(node->alloca.expr, dependencies, count);
    check(rc == 0, "Unable to add alloca expr dependencies");

    return 0;
    
error:
    qip_ast_node_dependencies_free(dependencies, count);
    return -1;
}


//--------------------------------------
// Validation
//--------------------------------------

// Validates the AST node.
//
// node - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_alloca_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    rc = qip_ast_node_validate(node->alloca.expr, module);
    check(rc == 0, "Unable to validate alloca expr");
    
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
int qip_ast_alloca_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bfromcstr("<alloca>\n"); check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->alloca.expr != NULL) {
        rc = qip_ast_node_dump(node->alloca.expr, ret);
        check(rc == 0, "Unable to dump expr");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
