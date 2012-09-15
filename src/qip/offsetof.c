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

// Creates an AST node for a offsetof invocation.
//
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_offsetof_create(qip_ast_node *var_ref)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_OFFSETOF;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->offsetof.var_ref = var_ref;
    if(var_ref != NULL) var_ref->parent = node;

    node->offsetof.return_type_ref = qip_ast_type_ref_create_cstr("Int");
    check_mem(node->offsetof.return_type_ref);
    node->offsetof.return_type_ref->parent = node;

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void qip_ast_offsetof_free(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_node_free(node->offsetof.var_ref);
        node->offsetof.var_ref = NULL;
        
        qip_ast_node_free(node->offsetof.return_type_ref);
        node->offsetof.return_type_ref = NULL;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_offsetof_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_offsetof_create(NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->offsetof.var_ref, &clone->offsetof.var_ref);
    check(rc == 0, "Unable to copy type");
    if(clone->offsetof.var_ref) clone->offsetof.var_ref->parent = clone;
    
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

// Recursively generates LLVM code for the variable reference AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_offsetof_codegen(qip_ast_node *node, qip_module *module,
                           LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_OFFSETOF, "Node type expected to be 'offsetof'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    qip_ast_node *var_ref = node->offsetof.var_ref;
    qip_ast_node *member  = var_ref->var_ref.member;

    // Retrieve the type of the variable reference.
    qip_ast_node *var_ref_type = NULL;
    rc = qip_ast_node_get_type(var_ref, module, &var_ref_type);
    check(rc == 0 && var_ref_type != NULL, "Unable to determine var ref type");

    // Retrieve the class AST for the variable reference.
    qip_ast_node *class = NULL;
    LLVMTypeRef llvm_type = NULL;
    rc = qip_module_get_type_ref(module, var_ref_type, &class, &llvm_type);
    check(rc == 0 && class != NULL && llvm_type != NULL, "Unable to find class: %s", bdata(var_ref_type->type_ref.name));

    // Retrieve property.
    int property_index = -1;
    rc = qip_ast_class_get_property_index(class, member->var_ref.name, &property_index);
    check(rc == 0 && property_index != -1, "Unable to find property '%s' on class '%s'", bdata(member->var_ref.name), bdata(class->class.name));

    // Create a null pointer.
    LLVMValueRef null_ptr = LLVMConstPointerNull(LLVMPointerType(llvm_type, 0));

    // Create a pointer to the member in the null pointer.
    LLVMValueRef member_ptr_value = LLVMBuildStructGEP(builder, null_ptr, property_index, "");
    
    // Cast pointer to an int and return.
    *value = LLVMBuildPtrToInt(builder, member_ptr_value, LLVMInt64TypeInContext(context), "");
    check((*value) != NULL, "Unable cast pointer to int");
    
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
int qip_ast_offsetof_preprocess(qip_ast_node *node, qip_module *module)
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
int qip_ast_offsetof_get_type(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_OFFSETOF, "Node type must be 'offsetof'");

    *ret = node->offsetof.return_type_ref;
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
int qip_ast_offsetof_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    rc = qip_ast_node_get_type_refs(node->offsetof.var_ref, type_refs, count);
    check(rc == 0, "Unable to add offsetof var ref");

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
int qip_ast_offsetof_get_dependencies(qip_ast_node *node,
                                    bstring **dependencies,
                                    uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Type ref.
    rc = qip_ast_node_get_dependencies(node->offsetof.var_ref, dependencies, count);
    check(rc == 0, "Unable to add offsetof dependencies");

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
int qip_ast_offsetof_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Type ref.
    rc = qip_ast_node_validate(node->offsetof.var_ref, module);
    check(rc == 0, "Unable to validate offsetof type ref");
    
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
int qip_ast_offsetof_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bfromcstr("<offsetof>\n"); check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->offsetof.var_ref != NULL) {
        rc = qip_ast_node_dump(node->offsetof.var_ref, ret);
        check(rc == 0, "Unable to dump type ref");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
