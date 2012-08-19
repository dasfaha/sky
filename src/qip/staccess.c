#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "llvm.h"
#include "node.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_staccess_codegen_property(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_staccess_codegen_method(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a struct member access.
//
// type        - The type of struct access to perform (property or method).
// var_name    - The name of the parent variable.
// member_name - The name of the member of the parent to access.
// args        - An array of argument values passed to a method.
// arg_count   - The number of arguments passed to a method.
// ret         - A pointer to where the ast node will be returned.
//
// Returns a structure member access node.
qip_ast_node *qip_ast_staccess_create(qip_ast_staccess_type_e type,
                                      qip_ast_node *var_ref,
                                      bstring member_name,
                                      qip_ast_node **args,
                                      unsigned int arg_count)
{
    qip_ast_node *node = malloc(sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_STACCESS;
    node->parent = NULL;
    node->line_no = node->char_no = 0;
    node->generated = false;

    node->staccess.type = type;

    node->staccess.member_name = bstrcpy(member_name);
    if(member_name != NULL) check_mem(node->staccess.member_name);

    node->staccess.var_ref = var_ref;
    if(var_ref) {
        var_ref->parent = node;
    }
    // Copy arguments.
    if(arg_count > 0) {
        node->staccess.args = malloc(sizeof(qip_ast_node*) * arg_count);
        check_mem(node->staccess.args);
        
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->staccess.args[i] = args[i];
            args[i]->parent = node;
        }
    }
    else {
        node->staccess.args = NULL;
    }
    node->staccess.arg_count = arg_count;

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a struct member access AST node from memory.
//
// node - The AST node to free.
void qip_ast_staccess_free(qip_ast_node *node)
{
    qip_ast_node_free(node->staccess.var_ref);
    
    if(node->staccess.member_name) bdestroy(node->staccess.member_name);
    node->staccess.member_name = NULL;

    unsigned int i;
    for(i=0; i<node->staccess.arg_count; i++) {
        qip_ast_node_free(node->staccess.args[i]);
        node->staccess.args[i] = NULL;
    }
    free(node->staccess.args);
    node->staccess.args = NULL;
    node->staccess.arg_count = 0;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_staccess_create(node->staccess.type, NULL, node->staccess.member_name, NULL, 0);
    check_mem(clone);

    rc = qip_ast_node_copy(node->staccess.var_ref, &clone->staccess.var_ref);
    check(rc == 0, "Unable to copy var ref");
    if(clone->staccess.var_ref) clone->staccess.var_ref->parent = clone;

    // Copy args.
    clone->staccess.arg_count = node->staccess.arg_count;
    clone->staccess.args = calloc(clone->staccess.arg_count, sizeof(*clone->staccess.args));
    check_mem(clone->staccess.args);
    for(i=0; i<clone->staccess.arg_count; i++) {
        rc = qip_ast_node_copy(node->staccess.args[i], &clone->staccess.args[i]);
        check(rc == 0, "Unable to copy arg");
        if(clone->staccess.args[i]) clone->staccess.args[i]->parent = clone;
    }
    
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

// Recursively generates LLVM code for the struct member access AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_codegen(qip_ast_node *node, qip_module *module,
                             LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    // Delegate codegen to access type specific functions.
    switch(node->staccess.type) {
        case QIP_AST_STACCESS_TYPE_PROPERTY: {
            rc = qip_ast_staccess_codegen_property(node, module, value);
            check(rc == 0, "Unable to codegen property access");
            break;
        }
        case QIP_AST_STACCESS_TYPE_METHOD: {
            rc = qip_ast_staccess_codegen_method(node, module, value);
            check(rc == 0, "Unable to codegen method invocation");
            break;
        }
    }

    return 0;

error:
    *value = NULL;
    return -1;
}

// Generates LLVM code for an access to a property on a struct.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_codegen_property(qip_ast_node *node, qip_module *module,
                                      LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(node->staccess.type == QIP_AST_STACCESS_TYPE_PROPERTY, "Access type expected to be 'property'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Retrieve a pointer to the member.
    LLVMValueRef ptr = NULL;
    rc = qip_ast_node_get_var_pointer(node, module, &ptr);
    check(rc == 0 && ptr != NULL, "Unable to retrieve struct member pointer");

    // Create load instruction.
    *value = LLVMBuildLoad(builder, ptr, "");
    check(*value != NULL, "Unable to create load instruction");

    return 0;

error:
    *value = NULL;
    return -1;
}

// Generates LLVM code for an access to a method on a struct.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_codegen_method(qip_ast_node *node, qip_module *module,
                                    LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(node->staccess.type == QIP_AST_STACCESS_TYPE_METHOD, "Access type expected to be 'method'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Find the caller reference.
    LLVMValueRef caller_pointer;
    rc = qip_ast_node_get_var_pointer(node->staccess.var_ref, module, &caller_pointer);
    check(rc == 0 && caller_pointer != NULL, "Unable to retrieve pointer to caller");

    // Retrieve the caller's type.
    bstring var_ref_type_name = NULL;
    rc = qip_ast_node_get_type_name(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0 && var_ref_type_name != NULL, "Unable to determine caller type");

    // If this is a complex type then dereference first.
    if(qip_llvm_is_double_pointer_type(LLVMTypeOf(caller_pointer))) {
        caller_pointer = LLVMBuildLoad(builder, caller_pointer, "");
    }

    // Determine function name.
    bstring function_name = bformat("%s.%s", bdata(var_ref_type_name), bdata(node->staccess.member_name));
    check_mem(function_name);
    
    // Retrieve function.
    LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    check(func != NULL, "Unable to find function: %s", bdata(function_name));
    check(LLVMCountParams(func) == node->staccess.arg_count+1, "Argument mismatch for '%s' (got %d, expected %d)", bdata(function_name), node->staccess.arg_count+1, LLVMCountParams(func));

    // Allocate arguments.
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * (node->staccess.arg_count+1));
    check_mem(args);
    args[0] = caller_pointer;

    // Evaluate arguments.
    unsigned int i;
    unsigned int arg_count = node->staccess.arg_count;
    for(i=0; i<arg_count; i++) {
        rc = qip_ast_node_codegen(node->staccess.args[i], module, &args[i+1]);
        check(rc == 0, "Unable to codegen argument: %d", i);
    }
    
    /*
    LLVMDumpValue(func);
    LLVMDumpValue(args[0]);
    if(arg_count > 0) LLVMDumpValue(args[1]);
    if(arg_count > 1) LLVMDumpValue(args[2]);
    */

    // Create call instruction.
    *value = LLVMBuildCall(builder, func, args, arg_count+1, "");
    
    return 0;

error:
    *value = NULL;
    return -1;
}

// Retrieves the pointer to the member of a struct.
//
// node    - The node to retrieve the pointer for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM pointer value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_get_pointer(qip_ast_node *node, qip_module *module,
                                 LLVMValueRef *value)
{
    int rc;

    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_STACCESS, "Node type expected to be 'struct member access'");
    check(module != NULL, "Module required");
    check(module->llvm_function != NULL, "Not currently in a function");
    check(node->staccess.var_ref != NULL, "Variable reference required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Find the struct reference.
    LLVMValueRef var_ref_pointer;
    rc = qip_ast_node_get_var_pointer(node->staccess.var_ref, module, &var_ref_pointer);
    check(rc == 0 && var_ref_pointer != NULL, "Unable to retrieve pointer to struct");

    // Retrieve the type of the variable reference.
    bstring var_ref_type_name = NULL;
    rc = qip_ast_node_get_type_name(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0 && var_ref_type_name != NULL, "Unable to determine struct type");

    // Retrieve the class AST for the variable reference.
    qip_ast_node *class_ast = NULL;
    rc = qip_module_get_type_ref(module, var_ref_type_name, &class_ast, NULL);
    check(rc == 0 && class_ast != NULL, "Unable to find class: %s", bdata(var_ref_type_name));

    // Determine the property index for the member.
    int property_index = 0;
    rc = qip_ast_class_get_property_index(class_ast, node->staccess.member_name, &property_index);
    check(rc == 0 && property_index >= 0, "Unable to find property '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

    // If this is a complex type from a function argument then dereference first.
    if(qip_llvm_is_double_pointer_type(LLVMTypeOf(var_ref_pointer))) {
        var_ref_pointer = LLVMBuildLoad(builder, var_ref_pointer, "");
    }

    // Build GEP instruction.
    *value = LLVMBuildStructGEP(builder, var_ref_pointer, property_index, "");
    check(*value != NULL, "Unable to build GEP instruction");

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
int qip_ast_staccess_preprocess(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Preprocess variable reference.
    rc = qip_ast_node_preprocess(node->staccess.var_ref, module);
    check(rc == 0, "Unable to preprocess struct access variable reference");

    // Preprocess arguments.
    uint32_t i;
    for(i=0; i<node->staccess.arg_count; i++) {
        rc = qip_ast_node_preprocess(node->staccess.args[i], module);
        check(rc == 0, "Unable to preprocess struct access arguments");
    }

    return 0;

error:
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node     - The AST node to determine the type for.
// module   - The compilation unit this node is a part of.
// type_ref - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_staccess_get_type(qip_ast_node *node, qip_module *module,
                              qip_ast_node **type_ref)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_STACCESS, "Node type must be 'struct member access'");

    // Retrieve the type of the variable reference.
    bstring var_ref_type_name = NULL;
    rc = qip_ast_node_get_type_name(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0 && var_ref_type_name != NULL, "Unable to determine struct type");

    // Retrieve the class AST for the variable reference.
    qip_ast_node *class_ast = NULL;
    rc = qip_module_get_type_ref(module, var_ref_type_name, &class_ast, NULL);
    check(rc == 0 && class_ast != NULL, "Unable to find class: %s", bdata(var_ref_type_name));

    // If this is a property access then grab the property type.
    if(node->staccess.type == QIP_AST_STACCESS_TYPE_PROPERTY) {
        // Find the property for the member.
        qip_ast_node *property = NULL;
        rc = qip_ast_class_get_property(class_ast, node->staccess.member_name, &property);
        check(rc == 0 || property != NULL, "Unable to find property '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

        // Return the property type name.
        *type_ref = property->property.var_decl->var_decl.type;
    }
    // If this is a method access then grab the return type of the method.
    else {
        // Find the method for the member.
        qip_ast_node *method = NULL;
        rc = qip_ast_class_get_method(class_ast, node->staccess.member_name, &method);
        check(rc == 0 || method != NULL, "Unable to find method '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

        // Return the method return type name.
        *type_ref = method->method.function->function.return_type;
    }

    return 0;
    
error:
    *type_ref = NULL;
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
int qip_ast_staccess_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    // Validate variable reference.
    rc = qip_ast_node_validate(node->staccess.var_ref, module);
    check(rc == 0, "Unable to validate struct access variable reference");

    // Retrieve the type of the variable reference.
    bstring var_ref_type_name = NULL;
    rc = qip_ast_node_get_type_name(node->staccess.var_ref, module, &var_ref_type_name);
    check(rc == 0, "Unable to determine struct type");
    
    if(var_ref_type_name != NULL) {
        // Retrieve the class AST for the variable reference.
        qip_ast_node *class_ast = NULL;
        rc = qip_module_get_type_ref(module, var_ref_type_name, &class_ast, NULL);
        check(rc == 0 && class_ast != NULL, "Unable to find class: %s", bdata(var_ref_type_name));

        // If this is a property access then validate the property exists.
        if(node->staccess.type == QIP_AST_STACCESS_TYPE_PROPERTY) {
            // Find the property for the member.
            qip_ast_node *property = NULL;
            rc = qip_ast_class_get_property(class_ast, node->staccess.member_name, &property);
            check(rc == 0, "Unable to find property on class");

            if(property == NULL) {
                msg = bformat("Unable to find property '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));
            }
            
            // Validate that the property referenced is not a Ref. This can be
            // ignored if the node is generated.
            if(!msg && biseqcstr(property->property.var_decl->var_decl.type->type_ref.name, "Ref") && !node->generated) {
                msg = bformat("Illegal reference to a Ref property '%s'", bdata(node->staccess.member_name));
            }
        }
        // If this is a method access then validate the method exists.
        else {
            // Find the method for the member.
            qip_ast_node *method = NULL;
            rc = qip_ast_class_get_method(class_ast, node->staccess.member_name, &method);
            check(rc == 0, "Unable to find method '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));

            if(method == NULL) {
                msg = bformat("Unable to find method '%s' on class '%s'", bdata(node->staccess.member_name), bdata(var_ref_type_name));
            }
        }
    }

    // Validate arguments.
    uint32_t i;
    for(i=0; i<node->staccess.arg_count; i++) {
        rc = qip_ast_node_validate(node->staccess.args[i], module);
        check(rc == 0, "Unable to validate struct access arguments");
    }

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
// Debugging
//--------------------------------------

// Append the contents of the AST node to the string.
// 
// node - The node to dump.
// ret  - A pointer to the bstring to concatenate to.
//
// Return 0 if successful, otherwise returns -1.s
int qip_ast_staccess_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    char *type_name = (node->staccess.type == QIP_AST_STACCESS_TYPE_PROPERTY ? "property" : "method");
    bstring str = bformat("<staccess member-name='%s' type='%s'>\n", bdatae(node->staccess.member_name, ""), type_name);
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->staccess.var_ref != NULL) {
        rc = qip_ast_node_dump(node->staccess.var_ref, ret);
        check(rc == 0, "Unable to dump variable reference");
    }

    // Dump arguments.
    uint32_t i;
    for(i=0; i<node->staccess.arg_count; i++) {
        rc = qip_ast_node_dump(node->staccess.args[i], ret);
        check(rc == 0, "Unable to dump struct access arguments");
    }

    return 0;

error:
    if(str) bdestroy(str);
    return -1;
}
