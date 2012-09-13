#include <stdlib.h>
#include "dbg.h"

#include "node.h"
#include "llvm.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_var_ref_codegen_member(qip_ast_node *node, qip_module *module,
    bool gen_ptr, LLVMValueRef parent_value, LLVMValueRef *value);

int qip_ast_var_ref_codegen_value(qip_ast_node *node, qip_module *module,
    bool gen_ptr, LLVMValueRef parent_value, LLVMValueRef *value);

int qip_ast_var_ref_codegen_invoke(qip_ast_node *node, qip_module *module,
    LLVMValueRef parent_value, LLVMValueRef *value);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a variable reference.
//
// type - The type of variable reference to create (value or invocation).
// name - The name of the variable value.
// ret  - A pointer to where the ast node will be returned.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create(qip_ast_var_ref_type_e type,
                                     bstring name)
{
    qip_ast_node *node = calloc(1, sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_VAR_REF;
    node->var_ref.type = type;
    node->var_ref.name = bstrcpy(name);
    check_mem(node->var_ref.name);

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Creates an AST node for a variable value reference.
//
// name - The name of the variable.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create_value(bstring name)
{
    return qip_ast_var_ref_create(QIP_AST_VAR_REF_TYPE_VALUE, name);
}

// Creates an AST node for a variable invocation.
//
// name      - The name of the variable.
// args      - An array of arguments to pass to the invocation.
// arg_count - The number of arguments.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create_invoke(bstring name, qip_ast_node **args,
                                            unsigned int arg_count)
{
    qip_ast_node *node = qip_ast_var_ref_create(QIP_AST_VAR_REF_TYPE_INVOKE, name);
    if(node) {
        // Copy arguments.
        if(arg_count > 0) {
            node->var_ref.args = malloc(sizeof(qip_ast_node*) * arg_count);
            check_mem(node->var_ref.args);
        }
    
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->var_ref.args[i] = args[i];
            args[i]->parent = node;
        }
        node->var_ref.arg_count = arg_count;
    }
    
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Creates a simple property access.
//
// name          - The name of the object to access.
// property_name - The name of the property to access.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create_property_access(bstring name,
                                                     bstring property_name)
{
    int rc;
    check(name != NULL, "Name required");
    check(property_name != NULL, "Property name required");
    
    // Create nodes.
    qip_ast_node *node = qip_ast_var_ref_create_value(name);
    check_mem(node);
    qip_ast_node *property_node = qip_ast_var_ref_create_value(property_name);
    check_mem(property_node);
    
    // Link the nodes.
    rc = qip_ast_var_ref_set_member(node, property_node);
    check(rc == 0, "Unable to assign property access member to node");
    
    return node;
    
error:
    qip_ast_node_free(node);
    qip_ast_node_free(property_node);
    return NULL;
}

// Creates a simple property access.
//
// name        - The name of the object to access.
// method_name - The name of the method to invoke.
// args        - The arguments to pass to the method.
// arg_count   - The number of arguments.
//
// Returns a variable reference node.
qip_ast_node *qip_ast_var_ref_create_method_invoke(bstring name,
                                                   bstring method_name,
                                                   qip_ast_node **args,
                                                   unsigned int arg_count)
{
    int rc;
    check(name != NULL, "Name required");
    check(method_name != NULL, "Method name required");
    check(args != NULL || arg_count == 0, "Method arguments required");
    
    // Create nodes.
    qip_ast_node *node = qip_ast_var_ref_create_value(name);
    check_mem(node);
    qip_ast_node *method_node = qip_ast_var_ref_create_invoke(method_name, args, arg_count);
    check_mem(method_node);
    
    // Link the nodes.
    rc = qip_ast_var_ref_set_member(node, method_node);
    check(rc == 0, "Unable to assign method invoke member to node");
    
    return node;
    
error:
    qip_ast_node_free(node);
    qip_ast_node_free(method_node);
    return NULL;
}

// Frees a variable reference AST node from memory.
//
// node - The AST node to free.
void qip_ast_var_ref_free(qip_ast_node *node)
{
    if(node != NULL) {
        bdestroy(node->var_ref.name);
        node->var_ref.name = NULL;

        uint32_t i;
        for(i=0; i<node->var_ref.arg_count; i++) {
            qip_ast_node_free(node->var_ref.args[i]);
            node->var_ref.args[i] = NULL;
        }
        free(node->var_ref.args);
        node->var_ref.args = NULL;
        node->var_ref.arg_count = 0;
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_var_ref_create(node->var_ref.type, node->var_ref.name);
    check_mem(clone);
    
    rc = qip_ast_node_copy(node->var_ref.member, &clone->var_ref.member);
    check(rc == 0, "Unable to copy member");
    if(clone->var_ref.member) clone->var_ref.member->parent = clone;

    // Copy args.
    clone->var_ref.arg_count = node->var_ref.arg_count;
    clone->var_ref.args = calloc(clone->var_ref.arg_count, sizeof(*clone->var_ref.args));
    check_mem(clone->var_ref.args);
    unsigned int i;
    for(i=0; i<clone->var_ref.arg_count; i++) {
        rc = qip_ast_node_copy(node->var_ref.args[i], &clone->var_ref.args[i]);
        check(rc == 0, "Unable to copy arg");
        if(clone->var_ref.args[i]) clone->var_ref.args[i]->parent = clone;
    }

    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Member Management
//--------------------------------------

// Sets the member access of this variable reference.
//
// node - The parent node.
// member - The member node that is being accessed.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_set_member(qip_ast_node *node, qip_ast_node *member)
{
    check(node != NULL, "Node required");
    
    if(node->var_ref.member) node->var_ref.member->parent = NULL;
    node->var_ref.member = member;
    if(node->var_ref.member) node->var_ref.member->parent = node;
    
    return 0;
    
error:
    return -1;
}

// Retrieves the last member in the variable reference chain.
//
// node - The parent node.
// ret  - The pointer where the last member should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_last_member(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");

    // Loop over all members until we reach the end.
    qip_ast_node *member = node;
    while(member->var_ref.member != NULL) {
        member = member->var_ref.member;
    }
    
    *ret = member;
    
    return 0;
    
error:
    return -1;
}

// Determines if a given node is a child member within a chain. This does not
// apply to the first member of a chain.
//
// node - The node.
// 
// Returns true if it is a child member, otherwise returns false.
bool qip_ast_var_ref_is_member(qip_ast_node *node)
{
    return (
        node->parent != NULL &&
        node->parent->type == QIP_AST_TYPE_VAR_REF &&
        node->parent->var_ref.member == node
        );
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the variable reference AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// gen_ptr - A flag stating if a pointer should be generated instead of a value.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_codegen(qip_ast_node *node, qip_module *module,
                            bool gen_ptr, LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'variable reference'");
    check(module != NULL, "Module required");

    // Initialize return value.
    *value = NULL;

    // Iterate down the member chain.
    LLVMValueRef parent_value = NULL;
    qip_ast_node *member = node;
    while(member != NULL) {
        // Only pass 'gen_ptr' for the last element.
        bool member_gen_ptr = (member->var_ref.member == NULL ? gen_ptr : false);
        
        // Codegen each member and pass the parent value.
        rc = qip_ast_var_ref_codegen_member(member, module, member_gen_ptr, parent_value, &parent_value);
        check(rc == 0, "Unable to codegen variable reference '%s'", bdata(member->var_ref.name));
        
        // Move to the next member in the chain.
        member = member->var_ref.member;
    }

    // Return last value generated.
    *value = parent_value;

    return 0;

error:
    *value = NULL;
    return -1;
}

// Generates LLVM code for a single member in a chain.
//
// node         - The node to generate an LLVM value for.
// module       - The compilation unit this node is a part of.
// parent_value - The value of the previous value in the chain.
// value        - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_codegen_member(qip_ast_node *node, qip_module *module,
                                   bool gen_ptr, LLVMValueRef parent_value,
                                   LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'variable reference'");
    check(module != NULL, "Module required");

    // Delegate codegen to access type specific functions.
    switch(node->var_ref.type) {
        case QIP_AST_VAR_REF_TYPE_VALUE: {
            rc = qip_ast_var_ref_codegen_value(node, module, gen_ptr, parent_value, value);
            check(rc == 0, "Unable to codegen value reference");
            break;
        }
        case QIP_AST_VAR_REF_TYPE_INVOKE: {
            rc = qip_ast_var_ref_codegen_invoke(node, module, parent_value, value);
            check(rc == 0, "Unable to codegen invocation");
            break;
        }
    }

    return 0;

error:
    *value = NULL;
    return -1;
}

// Generates LLVM code for an access to a value.
//
// node         - The node to generate an LLVM value for.
// module       - The compilation unit this node is a part of.
// parent_value - The value of the previous value in the chain.
// value        - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_codegen_value(qip_ast_node *node, qip_module *module,
                                  bool gen_ptr, LLVMValueRef parent_value,
                                  LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'var ref'");
    check(node->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE, "Access type expected to be 'value'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Initialize return value.
    *value = NULL;

    // Retrieve reference to member.
    rc = qip_ast_var_ref_get_pointer(node, module, parent_value, value);
    check(rc == 0, "Unable to retrieve pointer to variable reference");

    // Create load instruction unless this is pointer generation.
    if(!gen_ptr) {
        *value = LLVMBuildLoad(builder, *value, "");
        check(*value != NULL, "Unable to create load instruction");
    }

    return 0;

error:
    *value = NULL;
    return -1;
}

// Generates LLVM code for a function invocation.
//
// node         - The node to generate an LLVM value for.
// module       - The compilation unit this node is a part of.
// parent_value - The value of the previous value in the chain.
// value        - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_codegen_invoke(qip_ast_node *node, qip_module *module,
                                   LLVMValueRef parent_value,
                                   LLVMValueRef *value)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'var ref'");
    check(node->var_ref.type == QIP_AST_VAR_REF_TYPE_INVOKE, "Access type expected to be 'invoke'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Determine if this is a method invocation or a function invocation.
    bool is_method = qip_ast_var_ref_is_member(node);
    uint32_t total_arg_count = node->var_ref.arg_count + (is_method ? 1 : 0);
    
    // Retrieve pointer to function.
    LLVMValueRef func = NULL;
    rc = qip_ast_var_ref_get_pointer(node, module, parent_value, &func);
    check(rc == 0 && func != NULL, "Unable to retrieve function pointer");
    
    // Function references are actually function pointers. So if we have a
    // pointer to a function then it is actually a double pointer and we need
    // to unwrap it once.
    if(qip_llvm_is_double_pointer_type(LLVMTypeOf(func))) {
        func = LLVMBuildLoad(builder, func, "");
    }

    // Allocate arguments.
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * (node->var_ref.arg_count+1));
    check_mem(args);

    // Assign 'this' to first argument for method invocations.
    if(is_method) {
        args[0] = parent_value;
    }

    // Evaluate arguments.
    unsigned int i;
    unsigned int arg_count = node->var_ref.arg_count;
    for(i=0; i<arg_count; i++) {
        uint32_t index = i + (is_method ? 1 : 0);
        rc = qip_ast_node_codegen(node->var_ref.args[i], module, &args[index]);
        check(rc == 0, "Unable to codegen argument: %d", index);
    }

    /*
    LLVMDumpValue(func);
    if(total_arg_count > 0) LLVMDumpValue(args[0]);
    if(total_arg_count > 1) LLVMDumpValue(args[1]);
    if(total_arg_count > 2) LLVMDumpValue(args[2]);
    */

    // Create call instruction.
    *value = LLVMBuildCall(builder, func, args, total_arg_count, "");
    
    
    return 0;

error:
    *value = NULL;
    return -1;
}

// Retrieves a pointer to the member. For properties this will retrieve the
// address of the struct member. For methods this will retrieve the address of
// the function.
//
// node         - The node.
// module       - The compilation module.
// parent_value - The LLVM value generated by the parent.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_pointer(qip_ast_node *node, qip_module *module,
                                LLVMValueRef parent_value,
                                LLVMValueRef *value)
{
    int rc;
    bstring function_name = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // If the parent is a variable reference then use the parent value.
    if(qip_ast_var_ref_is_member(node)) {
        // Retrieve the parent variable reference's class type.
        qip_ast_node *class = NULL;
        rc = qip_ast_var_ref_get_class(node->parent, module, &class);
        check(rc == 0, "Unable to retrieve parent's class");
        check(class != NULL, "Class could not be found for variable: %s.%s", bdata(node->parent->var_ref.name), bdata(node->var_ref.name));

        // If this is a property access then retrieve the property address.
        if(node->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE) {
            // Determine the property index for the member.
            int property_index = 0;
            rc = qip_ast_class_get_property_index(class, node->var_ref.name, &property_index);
            check(rc == 0 && property_index >= 0, "Unable to find property '%s' on class '%s'", bdata(node->var_ref.name), bdata(class->class.name));

            // Build GEP instruction.
            //LLVMDumpValue(parent_value);
            *value = LLVMBuildStructGEP(builder, parent_value, property_index, "");
            check(*value != NULL, "Unable to build GEP instruction");
        }
        // Otherwise this is a method access so retrieve the method address.
        else {
            function_name = bformat("%s.%s", bdata(class->class.name), bdata(node->var_ref.name));
            *value = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
        }
    }
    // Otherwise this is referencing a local variable.
    else {
        rc = qip_module_get_variable(module, node->var_ref.name, NULL, value);
        check(rc == 0, "Unable to retrieve variable declaration: %s", bdata(node->var_ref.name));
    }
    
    bdestroy(function_name);
    return 0;
    
error:
    bdestroy(function_name);
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Preprocesses the node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_preprocess(qip_ast_node *node, qip_module *module,
                               qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    if(node->var_ref.member) {
        rc = qip_ast_node_preprocess(node->var_ref.member, module, stage);
        check(rc == 0, "Unable to preprocess member");
    }

    unsigned int i;
    for(i=0; i<node->var_ref.arg_count; i++) {
        rc = qip_ast_node_preprocess(node->var_ref.args[i], module, stage);
        check(rc == 0, "Unable to preprocess invoke arg var refs");
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
// node     - The node.
// module   - The module.
// type_ref - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_type(qip_ast_node *node, qip_module *module,
                             qip_ast_node **type_ref)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type must be 'var ref'");

    // Initialize return value.
    *type_ref = NULL;

    // If this is a member access then find the property/method type.
    if(qip_ast_var_ref_is_member(node)) {
        // Retrieve the class.
        qip_ast_node *class = NULL;
        rc = qip_ast_var_ref_get_class(node->parent, module, &class);
        check(rc == 0, "Unable to find class");

        // We may not have a class if this is reference off a dynamic property.
        if(class != NULL) {
            // If this is a value access then grab the property type.
            if(node->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE) {
                // Retrieve property.
                qip_ast_node *property = NULL;
                rc = qip_ast_class_get_property(class, node->var_ref.name, &property);
                check(rc == 0, "Unable to find property '%s' on class '%s'", bdata(node->var_ref.name), bdata(class->class.name));

                // Return the property type.
                if(property != NULL) {
                    *type_ref = property->property.var_decl->var_decl.type;
                }
            }
            // If this is a method access then grab the return type of the method.
            else {
                // Find the method for the member.
                qip_ast_node *method = NULL;
                rc = qip_ast_class_get_method(class, node->var_ref.name, &method);
                check(rc == 0, "Unable to find method '%s' on class '%s'", bdata(node->var_ref.name), bdata(class->class.name));

                // Return the method return type name.
                if(method != NULL) {
                    *type_ref = method->method.function->function.return_type;
                }
            }
        }
    }
    // Otherwise it's a local variable so search parents for a declaration.
    else {
        qip_ast_node *var_decl = NULL;
        rc = qip_ast_var_ref_get_var_decl(node, &var_decl);
        check(rc == 0 && var_decl != NULL, "Unable to find variable declaration for reference: %s", bdata(node->var_ref.name));
        *type_ref = var_decl->var_decl.type;
    }

    return 0;

error:
    *type_ref = NULL;
    return -1;
}

// Retrieves the AST class for the variable reference node.
//
// node   - The node.
// module - The module.
// ret    - A pointer to where the class should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_class(qip_ast_node *node, qip_module *module,
                              qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");
    
    // Initialize return value.
    *ret = NULL;
    
    // Retrieve the type of the variable reference.
    qip_ast_node *var_ref_type = NULL;
    rc = qip_ast_node_get_type(node, module, &var_ref_type);
    check(rc == 0 && var_ref_type != NULL, "Unable to determine var ref type");

    // Retrieve the class AST for the variable reference.
    rc = qip_module_get_type_ref(module, var_ref_type, ret, NULL);
    check(rc == 0, "Unable to find class: %s", bdata(var_ref_type->type_ref.name));

    return 0;

error:
    *ret = NULL;
    return -1;    
}

// Retrieves the AST node for the function that is being invoked by the
// reference. This could be a class method or it could be a regular function.
// If this is a method then only the function part of the method will be
// returned.
//
// node   - The node.
// module - The module.
// ret    - A pointer to where the function AST node should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_invoke_function(qip_ast_node *node,
                                        qip_module *module,
                                        qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'var ref'");
    check(node->var_ref.type == QIP_AST_VAR_REF_TYPE_INVOKE, "Var ref type expected to be 'invoke'");
    check(ret != NULL, "Return pointer required");

    // If this is a member access then find the property/method type.
    if(qip_ast_var_ref_is_member(node)) {
        // Retrieve the class.
        qip_ast_node *class = NULL;
        rc = qip_ast_var_ref_get_class(node->parent, module, &class);
        check(rc == 0 && class != NULL, "Unable to find class");

        // Find the method for the member.
        qip_ast_node *method = NULL;
        rc = qip_ast_class_get_method(class, node->var_ref.name, &method);
        check(rc == 0 || method != NULL, "Unable to find method '%s' on class '%s'", bdata(node->var_ref.name), bdata(class->class.name));

        // Return the method's function.
        *ret = method->method.function;
    }
    // Otherwise it's a local variable so search parents for a declaration.
    else {
        qip_ast_node *var_decl = NULL;
        rc = qip_ast_var_ref_get_var_decl(node, &var_decl);
        check(rc == 0 && var_decl != NULL, "Unable to find variable declaration for reference");
        
        *ret = var_decl->var_decl.initial_value;
    }
    
    return 0;

error:
    return -1;
}

// Retrieves the variable declaration for a variable reference.
//
// node - The node.
// ret  - A pointer to the matching variable declaration.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_var_decl(qip_ast_node *node, qip_ast_node **ret)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_VAR_REF, "Node type expected to be 'var ref'");
    check(!qip_ast_var_ref_is_member(node), "Node cannot be a var ref chain member");
    check(ret != NULL, "Return pointer required");

    // Initialize return value.
    *ret = NULL;
    
    // Search parent hierarchy for matching variable declaration.
    qip_ast_node *var_decl = NULL;
    qip_ast_node *parent = node->parent;
    while(parent != NULL) {
        int rc = qip_ast_node_get_var_decl(parent, node->var_ref.name, &var_decl);
        check(rc == 0, "Unable to search node for variable declarations");

        // If a declaration was found then return its type.
        if(var_decl != NULL) {
            *ret = var_decl;
            break;
        }

        parent = parent->parent;
    }
    
    return 0;

error:
    *ret = NULL;
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
int qip_ast_var_ref_validate(qip_ast_node *node, qip_module *module)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    
    return 0;

error:
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_var_refs(qip_ast_node *node, bstring name,
                                 qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    // Only append top-level variable reference in a chain.
    if(!qip_ast_var_ref_is_member(node) && biseq(node->var_ref.name, name) == 1) {
        rc = qip_array_push(array, (void*)node);
        check(rc == 0, "Unable to add variable reference to array");
    }

    if(node->var_ref.member) {
        rc = qip_ast_node_get_var_refs(node->var_ref.member, name, array);
        check(rc == 0, "Unable to retrieve member var refs");
    }

    unsigned int i;
    for(i=0; i<node->var_ref.arg_count; i++) {
        rc = qip_ast_node_get_var_refs(node->var_ref.args[i], name, array);
        check(rc == 0, "Unable to retrieve invoke arg var refs");
    }

    return 0;
    
error:
    return -1;
}

// Retrieves all variable reference of a given type name within this node.
//
// node      - The node.
// module    - The module.
// type_name - The type name.
// array     - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_var_ref_get_var_refs_by_type(qip_ast_node *node, qip_module *module,
                                         bstring type_name, qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_name != NULL, "Type name required");
    check(array != NULL, "Array required");

    // Retrieve the variable type.
    qip_ast_node *type_ref = NULL;
    rc = qip_ast_var_ref_get_type(node, module, &type_ref);
    check(rc == 0, "Unable to retrieve type");

    // If the root type matches the name then add it to the array.
    if(type_ref && biseq(type_ref->type_ref.name, type_name)) {
        rc = qip_array_push(array, (void*)node);
        check(rc == 0, "Unable to add variable reference to array");
    }
    
    if(node->var_ref.member) {
        rc = qip_ast_node_get_var_refs_by_type(node->var_ref.member, module, type_name, array);
        check(rc == 0, "Unable to retrieve member var refs by type");
    }

    unsigned int i;
    for(i=0; i<node->var_ref.arg_count; i++) {
        rc = qip_ast_node_get_var_refs_by_type(node->var_ref.args[i], module, type_name, array);
        check(rc == 0, "Unable to retrieve invoke arg var refs by type");
    }

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
int qip_ast_var_ref_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    char *type_name = (node->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE ? "value" : "invoke");
    bstring str = bformat("<var-ref type='%s' name='%s'>\n", type_name, bdatae(node->var_ref.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children.
    if(node->var_ref.member != NULL) {
        rc = qip_ast_node_dump(node->var_ref.member, ret);
        check(rc == 0, "Unable to dump member");
    }

    // Dump arguments.
    uint32_t i;
    for(i=0; i<node->var_ref.arg_count; i++) {
        rc = qip_ast_node_dump(node->var_ref.args[i], ret);
        check(rc == 0, "Unable to dump arguments");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
