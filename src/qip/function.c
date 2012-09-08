#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "array.h"
#include "llvm.h"
#include "node.h"
#include "util.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int qip_ast_function_codegen_prototype(qip_ast_node *node, qip_module *module,
    LLVMValueRef *value);

int qip_ast_function_generate_external_call(qip_ast_node *node,
    qip_module *module, LLVMBasicBlockRef block);

int qip_ast_function_bind(qip_ast_node *node, qip_module *module);

int qip_ast_function_bind_var_decl(qip_ast_node *node, qip_module *module,
    qip_ast_node *var_decl);

int qip_ast_function_bind_apply(qip_ast_node *node, qip_module *module,
    qip_ast_node *target_node);

int qip_ast_function_bind_with_type_ref(qip_ast_node *node, qip_module *module,
    qip_ast_node *type_ref);

    
//==============================================================================
//
// Functions
//
//==============================================================================

// Creates an AST node for a function.
//
// name        - The name of the function.
// return_type - The data type that the function returns.
// args        - The arguments of the function.
// arg_count   - The number of arguments the function has.
// body        - The contents of the function.
// ret         - A pointer to where the ast node will be returned.
//
// Returns a function node.
qip_ast_node *qip_ast_function_create(bstring name, qip_ast_node *return_type,
                            struct qip_ast_node **args, unsigned int arg_count,
                            struct qip_ast_node *body)
{
    qip_ast_node *node = calloc(1, sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_FUNCTION;
    node->function.bound = true;
    node->function.name = bstrcpy(name);
    if(name) check_mem(node->function.name);
    
    node->function.return_type = return_type;
    if(return_type != NULL) return_type->parent = node;

    // Copy arguments.
    if(arg_count > 0) {
        size_t sz = sizeof(qip_ast_node*) * arg_count;
        node->function.args = malloc(sz);
        check_mem(node->function.args);
        
        unsigned int i;
        for(i=0; i<arg_count; i++) {
            node->function.args[i] = args[i];
            args[i]->parent = node;
        }
    }
    else {
        node->function.args = NULL;
    }
    node->function.arg_count = arg_count;
    
    // Assign function body.
    node->function.body = body;
    if(body != NULL) {
        body->parent = node;
        body->block.name = NULL;
    }

    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees a function AST node from memory.
//
// node - The AST node to free.
void qip_ast_function_free(struct qip_ast_node *node)
{
    if(node->function.name) bdestroy(node->function.name);
    node->function.name = NULL;

    qip_ast_node_free(node->function.return_type);
    node->function.return_type = NULL;
    
    if(node->function.arg_count > 0) {
        unsigned int i;
        for(i=0; i<node->function.arg_count; i++) {
            qip_ast_node_free(node->function.args[i]);
            node->function.args[i] = NULL;
        }
        free(node->function.args);
        node->function.arg_count = 0;
    }

    if(node->function.body) qip_ast_node_free(node->function.body);
    node->function.body = NULL;
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    unsigned int i;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *clone = qip_ast_function_create(node->function.name, NULL, NULL, 0, NULL);
    check_mem(clone);

    rc = qip_ast_node_copy(node->function.return_type, &clone->function.return_type);
    check(rc == 0, "Unable to copy return type");
    if(clone->function.return_type) clone->function.return_type->parent = clone;
    
    // Copy args.
    clone->function.arg_count = node->function.arg_count;
    clone->function.args = calloc(clone->function.arg_count, sizeof(*clone->function.args));
    check_mem(clone->function.args);
    for(i=0; i<clone->function.arg_count; i++) {
        rc = qip_ast_node_copy(node->function.args[i], &clone->function.args[i]);
        check(rc == 0, "Unable to copy arg");
        if(clone->function.args[i]) clone->function.args[i]->parent = clone;
    }

    rc = qip_ast_node_copy(node->function.body, &clone->function.body);
    check(rc == 0, "Unable to copy body");
    if(clone->function.body) clone->function.body->parent = clone;
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Argument Management
//--------------------------------------

// Adds an argument to a function.
//
// node - The function node.
// farg - The argument to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_add_arg(qip_ast_node *node,
                             qip_ast_node *farg)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(farg != NULL, "Argument is required");
    
    // Append argument to function.
    node->function.arg_count++;
    node->function.args = realloc(node->function.args, sizeof(qip_ast_node*) * node->function.arg_count);
    check_mem(node->function.args);
    node->function.args[node->function.arg_count-1] = farg;
    
    // Link property to function.
    farg->parent = node;
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the function AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen(qip_ast_node *node, qip_module *module,
                             LLVMValueRef *value)
{
    int rc;

    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Create function prototype.
    LLVMValueRef func = NULL;
    rc = qip_ast_function_codegen_prototype(node, module, &func);
    check(rc == 0, "Unable to generate function prototype");
    
    // Store the current function on the module.
    qip_scope *scope = qip_scope_create_function(func); check_mem(scope);
    rc = qip_module_push_scope(module, scope);
    check(rc == 0, "Unable to add function scope");

    // Grab the External metadata node if there is one.
    qip_ast_node *external_metadata_node = NULL;
    rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
    check(rc == 0, "Unable to retrieve external metadata node");
    
    // Generate basic block for body.
    LLVMBasicBlockRef block = LLVMAppendBasicBlock(scope->llvm_function, "");

    // Generate function arguments.
    LLVMPositionBuilderAtEnd(builder, block);
    rc = qip_ast_function_codegen_args(node, module);
    check(rc == 0, "Unable to codegen function arguments");

    // If this function is marked as external then dynamically generate the
    // code to call the function and return the value.
    if(external_metadata_node != NULL) {
        rc = qip_ast_function_generate_external_call(node, module, block);
        check(rc == 0, "Unable to generate call to external function");
    }
    // Otherwise generate the body if it exists.
    else if(node->function.body != NULL) {
        rc = qip_ast_block_codegen_with_block(node->function.body, module, block);
        check(rc == 0, "Unable to generate function body statements");

        // If this is a void return type and there is no explicit return then
        // add one before the end.
        if(qip_ast_type_ref_is_void(node->function.return_type)) {
            if(node->function.body->block.expr_count > 0 &&
               node->function.body->block.exprs[node->function.body->block.expr_count-1]->type != QIP_AST_TYPE_FRETURN)
            {
                LLVMBuildRetVoid(builder);
            }
        }
    }
    // If there's no body or it's not external then we have a problem.
    else {
        sentinel("Function must be external or have a body");
    }
    
    // Dump before verification.
    //LLVMDumpValue(func);
    
    // Verify function.
    rc = LLVMVerifyFunction(func, LLVMPrintMessageAction);
    check(rc != 1, "Invalid function");

    // Unset the current function.
    rc = qip_module_pop_scope(module);
    check(rc == 0, "Unable to remove function scope");
    if(scope->llvm_last_alloca != NULL) {
        LLVMInstructionEraseFromParent(scope->llvm_last_alloca);
        scope->llvm_last_alloca = NULL;
    }

    // Reset the builder position at the end of the new function scope if
    // one still exists.
    qip_scope *new_scope = NULL;
    rc = qip_module_get_current_function_scope(module, &new_scope);
    check(rc == 0, "Unable to retrieve new function scope");
    
    if(new_scope != NULL) {
        LLVMPositionBuilderAtEnd(builder, LLVMGetLastBasicBlock(new_scope->llvm_function));
    }

    // Return function as a value.
    *value = func;
    
    return 0;

error:
    //if(func) LLVMDeleteFunction(func);
    *value = NULL;
    return -1;
}

// Generates a function call to an external function.
//
// node  - The function node.
// block - The LLVM block the call is being added to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_generate_external_call(qip_ast_node *node,
                                             qip_module *module,
                                             LLVMBasicBlockRef block)
{
    int rc;
    check(node != NULL, "Node required");
    check(block != NULL, "Block required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Grab the External metadata node if there is one.
    qip_ast_node *external_metadata_node = NULL;
    rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
    check(rc == 0, "Unable to retrieve external metadata node");
    check(external_metadata_node != NULL, "External metadata node does not exist");
    
    // Retrieve function name from metadata.
    bstring function_name = NULL;
    rc = qip_ast_metadata_get_item_value(external_metadata_node, NULL, &function_name);
    check(rc == 0, "Unable to retrieve function name from metadata");
    
    // Make sure we have an external function name.
    check(function_name != NULL, "External function name required");
    
    // Loop over function arguments to make call arguments.
    unsigned int i;
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * node->function.arg_count); check_mem(args);
    for(i=0; i<node->function.arg_count; i++) {
        qip_ast_node *farg = node->function.args[i];
        bstring arg_name = farg->farg.var_decl->var_decl.name;
        
        // Retrieve LLVM reference.
        qip_ast_node *farg_var_decl = NULL;
        LLVMValueRef farg_value = NULL;
        rc = qip_module_get_variable(module, arg_name, &farg_var_decl, &farg_value);
        check(rc == 0, "Unable to retrieve function argument declaration: %s", bdata(arg_name));
        check(farg_value != NULL, "No LLVM value for function argument declaration: %s", bdata(arg_name));
        
        // Load the reference and add it to the argument list.
        args[i] = LLVMBuildLoad(builder, farg_value, "");
        check(args[i] != NULL, "Unable to build load for function argument");
    }
    
    // Retrieve function.
    LLVMValueRef func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    check(func != NULL, "Unable to find external function: %s", bdata(function_name));
    check(LLVMCountParams(func) == node->function.arg_count, "Argument mismatch (got %d, expected %d)", node->function.arg_count, LLVMCountParams(func));
    
    // Create call instruction.
    LLVMValueRef call_value = LLVMBuildCall(builder, func, args, node->function.arg_count, "");
    check(call_value != NULL, "Unable to build external function call");
    
    // If function return void then generate a void return.
    if(qip_ast_type_ref_is_void(node->function.return_type)) {
        LLVMValueRef ret_value = LLVMBuildRetVoid(builder);
        check(ret_value != NULL, "Unable to build external void return");
    }
    // Otherwise return the value from the function.
    else {
        LLVMValueRef ret_value = LLVMBuildRet(builder, call_value);
        check(ret_value != NULL, "Unable to build external return value");
    }
    
    return 0;

error:
    return -1;
}

// Generates the function prototype. This is used for forward declarations
// and for the function codegen itself.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_prototype(qip_ast_node *node, qip_module *module,
                                       LLVMValueRef *value)
{
    int rc;
    
    // Retrieve the fully qualified function name.
    bstring qualified_name = NULL;
    rc = qip_ast_function_get_qualified_name(node, &qualified_name);
    check(rc == 0, "Unable to determine fully qualified function name");
    
    // Generate the prototype with the given name.
    rc = qip_ast_function_codegen_prototype_with_name(node, module, qualified_name, value);
    check(rc == 0, "Unable to generate named prototype");
    
    bdestroy(qualified_name);
    return 0;

error:
    bdestroy(qualified_name);
    return -1;
}

// Generates the function prototype but overrides the name. This is used by
// the metadata to create external APIs to C.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_prototype_with_name(qip_ast_node *node,
                                                 qip_module *module,
                                                 bstring function_name,
                                                 LLVMValueRef *value)
{
    int rc;
    unsigned int i;
    LLVMValueRef func = NULL;
    bstring arg_type_name = NULL;
    bstring return_type_name = NULL;

    // Check for an existing prototype.
    func = LLVMGetNamedFunction(module->llvm_module, bdata(function_name));
    
    // If a prototype exists then simply verify it matches and return it.
    if(func != NULL) {
        check(LLVMCountBasicBlocks(func) == 0, "Illegal function redefinition");
        check(LLVMCountParams(func) == node->function.arg_count, "Function prototype already exists with different arguments");
    }
    // If there is no prototype then create one.
    else {
        // Dynamically generate the return type of the function if it is missing.
        if(node->function.return_type == NULL) {
            rc = qip_ast_function_generate_return_type(node, module);
            check(rc == 0, "Unable to generate return type for function");
        }

        // Create a list of function argument types.
        qip_ast_node *arg;
        unsigned int arg_count = node->function.arg_count;
        LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef) * arg_count);

        // Create arguments.
        for(i=0; i<arg_count; i++) {
            qip_ast_node *arg = node->function.args[i];
            LLVMTypeRef param = NULL;
            rc = qip_module_get_type_ref(module, arg->farg.var_decl->var_decl.type, NULL, &param);
            check(rc == 0, "Unable to determine function argument type");
        
            // Pass argument as reference if this is a complex type.
            if(qip_llvm_is_complex_type(param)) {
                params[i] = LLVMPointerType(param, 0);
            }
            // Otherwise pass it by value.
            else {
                params[i] = param;
            }
        }

        // Determine return type.
        LLVMTypeRef return_type;
        rc = qip_module_get_type_ref(module, node->function.return_type, NULL, &return_type);
        check(rc == 0, "Unable to determine function return type");

        if(qip_llvm_is_complex_type(return_type)) {
            return_type = LLVMPointerType(return_type, 0);
        }

        // Create function type.
        LLVMTypeRef funcType = LLVMFunctionType(return_type, params, arg_count, false);
        check(funcType != NULL, "Unable to create function type");

        // Create function.
        func = LLVMAddFunction(module->llvm_module, bdata(function_name), funcType);
        check(func != NULL, "Unable to create function");
    
        // Assign names to function arguments.
        for(i=0; i<arg_count; i++) {
            arg = node->function.args[i];
            LLVMValueRef param = LLVMGetParam(func, i);
            LLVMSetValueName(param, bdata(arg->farg.var_decl->var_decl.name));
        }
    }
    
    // Return function prototype;
    *value = func;
    
    bdestroy(arg_type_name);
    bdestroy(return_type_name);
    return 0;

error:
    bdestroy(arg_type_name);
    bdestroy(return_type_name);
    if(func) LLVMDeleteFunction(func);
    *value = NULL;
    return -1;
}

// Generates the allocas for the function arguments. This has to be called
// from the block since that is where the entry block is created.
//
// node    - The function node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_args(qip_ast_node *node, qip_module *module)
{
    int rc;
    unsigned int i;
    
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type expected to be 'function'");
    check(module != NULL, "Module required");

    LLVMBuilderRef builder = module->compiler->llvm_builder;
    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

    // Retrieve current function scope.
    qip_scope *scope = NULL;
    rc = qip_module_get_current_function_scope(module, &scope);
    check(rc == 0 && scope != NULL, "Unable to retrieve current function scope");

    // Codegen allocas.
    LLVMValueRef *values = malloc(sizeof(LLVMValueRef) * node->function.arg_count);
    check_mem(values);
    
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_codegen(node->function.args[i], module, &values[i]);
        check(rc == 0, "Unable to determine function argument type");
    }
    
    scope->llvm_last_alloca = LLVMBuildAlloca(builder, LLVMInt1TypeInContext(context), "nop");
    
    // Codegen store instructions.
    for(i=0; i<node->function.arg_count; i++) {
        LLVMValueRef param = LLVMGetParam(scope->llvm_function, i);
        LLVMValueRef build_value = LLVMBuildStore(builder, param, values[i]);
        check(build_value != NULL, "Unable to create store instruction");
    }
    
    free(values);

    return 0;
    
error:
    if(values) free(values);
    return -1;
}

// Recursively generates forward declarations.
//
// node    - The node.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_codegen_forward_decl(qip_ast_node *node,
                                          qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(module != NULL, "Module required");

    // Generate forward declaration.
    LLVMValueRef func = NULL;
    rc = qip_ast_function_codegen_prototype(node, module, &func);
    check(rc == 0, "Unable to generate function prototype");
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Preprocessor
//--------------------------------------

// Processes the node.
//
// node   - The node.
// module - The module that the node is a part of.
// stage  - The processing stage.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_preprocess(qip_ast_node *node, qip_module *module,
                                qip_ast_processing_stage_e stage)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Preprocess argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_preprocess(node->function.args[i], module, stage);
        check(rc == 0, "Unable to preprocess function argument type");
    }

    // Preprocess block.
    if(node->function.body != NULL) {
        rc = qip_ast_node_preprocess(node->function.body, module, stage);
        check(rc == 0, "Unable to preprocess function body");
    }

    if(stage == QIP_AST_PROCESSING_STAGE_INITIALIZED) {
        // If this is a late binding function then apply arg and return types
        // once all modules have been loaded.
        if(!node->function.bound) {
            rc = qip_ast_function_bind(node, module);
            check(rc == 0, "Unable to bind function");
        }
    }

    return 0;

error:
    return -1;   
}


//--------------------------------------
// Misc
//--------------------------------------

// Retrieves the class that this function belongs to (if it is a method).
// Otherwise it returns NULL as the class.
//
// node      - The function AST node.
// class_ast - A pointer to where the class AST node should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_class(qip_ast_node *node, qip_ast_node **class_ast)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(class_ast != NULL, "Class return pointer must not be null");
    
    *class_ast = NULL;

    // Check if there is a parent method.
    if(node->parent != NULL && node->parent->type == QIP_AST_TYPE_METHOD) {
        qip_ast_node *method = node->parent;

        // Check if the method has a class.
        if(method->parent != NULL && method->parent->type == QIP_AST_TYPE_CLASS) {
            *class_ast = method->parent;
        }
    }
    
    return 0;
    
error:
    *class_ast = NULL;
    return -1;
}


// Updates the return type of the function based on the last return statement
// of the function. This is used for implicit functions like the main function
// of a module.
//
// node   - The function ast node to generate a type for.
// module - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_generate_return_type(qip_ast_node *node,
                                          qip_module *module)
{
    int rc;
    qip_ast_node *type = NULL;
    
    check(node != NULL, "Function required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");

    // If function has no body then its return type is void.
    qip_ast_node *body = node->function.body;
    if(body == NULL) {
        node->function.return_type = qip_ast_type_ref_create_cstr("void");
        check_mem(node->function.return_type);
        node->function.return_type->parent = node;
    }
    // Otherwise find the last return statement and determine its type.
    else {
        qip_ast_node *freturn = NULL;
        
        // Loop over all returns and save the last one.
        unsigned int i;
        for(i=0; i<body->block.expr_count; i++) {
            if(body->block.exprs[i]->type == QIP_AST_TYPE_FRETURN) {
                freturn = body->block.exprs[i];
            }
        }
        
        // If there is no return statement or it's a void return then the type
        // is void.
        if(freturn == NULL || freturn->freturn.value == NULL) {
            type = qip_ast_type_ref_create_cstr("void"); check_mem(type);
        }
        // Otherwise check the last return value to determine its type.
        else {
            // Retrieve the last variable reference in a chain to determine the type.
            qip_ast_node *target_node = freturn->freturn.value;
            if(target_node->type == QIP_AST_TYPE_VAR_REF) {
                rc = qip_ast_var_ref_get_last_member(target_node, &target_node);
                check(rc == 0, "Unable to retrieve last member of return var ref");
            }
            
            rc = qip_ast_node_get_type(target_node, module, &type);
            check(rc == 0, "Unable to determine return type");
        }

        // Copy return type.
        rc = qip_ast_node_copy(type, &node->function.return_type);
        check(rc == 0, "Unable to copy type");
        node->function.return_type->parent = node;
    }

    return 0;
    
error:
    return -1;
}

// Searches for variable declarations within the function's argument list.
//
// node     - The node to search within.
// name     - The name of the variable to search for.
// var_decl - A pointer to where the variable declaration should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_var_decl(qip_ast_node *node, bstring name,
                                  qip_ast_node **var_decl)
{
    unsigned int i;
    
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");

    // Search argument list for variable declaration.
    *var_decl = NULL;
    for(i=0; i<node->function.arg_count; i++) {
        if(biseq(node->function.args[i]->farg.var_decl->var_decl.name, name) == 1) {
            *var_decl = node->function.args[i]->farg.var_decl;
            break;
        }
    }

    return 0;
    
error:
    *var_decl = NULL;
    return -1;    
}

// Retrieves the fully qualified name for a function.
//
// node - The node.
// name - A pointer to where the fully qualified name should be set.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_qualified_name(qip_ast_node *node, bstring *name)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Name return pointer required");

    // Initialize return value.
    if(name) *name = NULL;
    
    // Only generate a name if this is not an anonymous function.
    if(node->function.name != NULL) {
        // Find the class this function belongs to, if any.
        qip_ast_node *class_ast = NULL;
        rc = qip_ast_function_get_class(node, &class_ast);
        check(rc == 0, "Unable to retrieve parent class for function");

        // Function name should be prepended with the class name if this is a method.
        bool is_method = (class_ast != NULL);
        if(is_method) {
            check(blength(class_ast->class.name) > 0, "Class name required for method");
            *name = bformat("%s.%s", bdata(class_ast->class.name), bdata(node->function.name));
            check_mem(*name);
        }
        else {
            *name = bstrcpy(node->function.name);
            check_mem(*name);
        }
    }
    else {
        *name = bfromcstr("");
    }

    return 0;

error:
    if(name) *name = NULL;
    return -1;
}

//--------------------------------------
// Metadata
//--------------------------------------

// Retrieves the external metadata node for a function.
//
// node - The function node.
// external_metadata_node - A pointer to where the metadata node should be
//                          returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_external_metadata_node(qip_ast_node *node,
                                                qip_ast_node **external_metadata_node)
{
    int rc;
    check(node != NULL, "Node required");
    check(external_metadata_node != NULL, "External Node return pointer required");
    
    struct tagbstring metadata_name = bsStatic("External");

    // Initialize return value.
    *external_metadata_node = NULL;
    
    // Check if the function is attached to a method.
    if(node->parent != NULL && node->parent->type == QIP_AST_TYPE_METHOD) {
        // Retrieve metadata from method.
        rc = qip_ast_method_get_metadata_node(node->parent, &metadata_name, external_metadata_node);
        check(rc == 0, "Unable to retrieve metadata from method");
    }
    
    return 0;

error:
    *external_metadata_node = NULL;
    return -1;
}


//--------------------------------------
// Find
//--------------------------------------

// Computes a list of type refs used by the node.
//
// node      - The node.
// type_refs - A pointer to an array of type refs.
// count     - A pointer to where the number of type refs is stored.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_type_refs(qip_ast_node *node,
                                   qip_ast_node ***type_refs,
                                   uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(type_refs != NULL, "Type refs return pointer required");
    check(count != NULL, "Type ref count return pointer required");

    // Add return type.
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_get_type_refs(node->function.return_type, type_refs, count);
        check(rc == 0, "Unable to add function return type refs");
    }

    // Add argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_get_type_refs(node->function.args[i], type_refs, count);
        check(rc == 0, "Unable to add function argument type type refs");
    }

    // Add block type refs.
    if(node->function.body != NULL) {
        rc = qip_ast_node_get_type_refs(node->function.body, type_refs, count);
        check(rc == 0, "Unable to add function body type refs");
    }

    return 0;
    
error:
    qip_ast_node_type_refs_free(type_refs, count);
    return -1;
}

// Retrieves all variable reference of a given name within this node.
//
// node  - The node.
// name  - The variable name.
// array - The array to add the references to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_get_var_refs(qip_ast_node *node, bstring name,
                                  qip_array *array)
{
    int rc;
    check(node != NULL, "Node required");
    check(name != NULL, "Variable name required");
    check(array != NULL, "Array required");

    if(node->function.return_type != NULL) {
        rc = qip_ast_node_get_var_refs(node->function.return_type, name, array);
        check(rc == 0, "Unable to add function return var refs");
    }

    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_get_var_refs(node->function.args[i], name, array);
        check(rc == 0, "Unable to add function argument var refs");
    }

    if(node->function.body != NULL) {
        rc = qip_ast_node_get_var_refs(node->function.body, name, array);
        check(rc == 0, "Unable to add function body var refs");
    }

    return 0;
    
error:
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
int qip_ast_function_get_dependencies(qip_ast_node *node,
                                      bstring **dependencies,
                                      uint32_t *count)
{
    int rc;
    check(node != NULL, "Node required");
    check(dependencies != NULL, "Dependencies return pointer required");
    check(count != NULL, "Dependency count return pointer required");

    // Add return type.
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_get_dependencies(node->function.return_type, dependencies, count);
        check(rc == 0, "Unable to add function return type dependencies");
    }

    // Add argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_get_dependencies(node->function.args[i], dependencies, count);
        check(rc == 0, "Unable to add function argument type dependency");
    }

    // Add block dependencies.
    if(node->function.body != NULL) {
        rc = qip_ast_node_get_dependencies(node->function.body, dependencies, count);
        check(rc == 0, "Unable to add function body dependencies");
    }

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
// node   - The node to validate.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_validate(qip_ast_node *node, qip_module *module)
{
    int rc;
    bstring msg = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Retrieve parent.
    qip_ast_node *method = (node->parent && node->parent->type == QIP_AST_TYPE_METHOD ? node->parent : NULL);

    // If there is no body then the function must have an External metatag.
    if(method != NULL && node->function.body == NULL) {
        // Grab the External metadata node if there is one.
        qip_ast_node *external_metadata_node = NULL;
        rc = qip_ast_function_get_external_metadata_node(node, &external_metadata_node);
        check(rc == 0, "Unable to retrieve external metadata node");
        
        if(external_metadata_node == NULL) {
            msg = bformat("Function '%s' must have a body unless it is marked as external", bdata(node->function.name));
            rc = qip_module_add_error(module, node, msg);
            check(rc == 0, "Unable to add module error");
            bdestroy(msg);
            msg = NULL;
        }
    }

    // Validate argument types.
    uint32_t i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_validate(node->function.args[i], module);
        check(rc == 0, "Unable to validate function argument type");
    }

    // Validate block.
    if(node->function.body != NULL) {
        rc = qip_ast_node_validate(node->function.body, module);
        check(rc == 0, "Unable to validate function body");
    }

    bdestroy(msg);
    return 0;

error:
    bdestroy(msg);
    return -1;   
}


//--------------------------------------
// Binding
//--------------------------------------

// Performs binding of an unbound function. This involves applying argument
// and return types everywhere the function is used.
//
// node   - The node.
// module - The module that the node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_bind(qip_ast_node *node, qip_module *module)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    // Ignore if the function is already bound.
    if(node->function.bound) {
        return 0;
    }
    
    // If function is assigned to a variable then apply binding to all
    // references to the variable.
    if(node->parent && node->parent->type == QIP_AST_TYPE_VAR_DECL) {
        rc = qip_ast_function_bind_var_decl(node, module, node->parent);
        check(rc == 0, "Unable to bind to variable");
    }
    // Otherwise apply the binding in-place.
    else {
        rc = qip_ast_function_bind_apply(node, module, node);
        check(rc == 0, "Unable to apply in-place binding");
    }

    return 0;

error:
    return -1;
}

// Performs late binding of a function to all references of a variable.
//
// node     - The node.
// module   - The module that the node is a part of.
// var_decl - The variable declaration that the function is bound to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_bind_var_decl(qip_ast_node *node, qip_module *module,
                                   qip_ast_node *var_decl)
{
    int rc;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(var_decl != NULL, "Variable declaration required");
    
    // Retrieve the block the variable declaration is defined in.
    qip_ast_node *block = var_decl->parent;
    check(block != NULL, "Variable declaration must have a parent");
    check(block->type == QIP_AST_TYPE_BLOCK, "Variable declaration must be defined in a block: '%s'", bdata(var_decl->var_decl.name));
    
    // Find all references to the variable.
    qip_array *array = qip_array_create(); check_mem(array);
    rc = qip_ast_block_get_var_refs(block, var_decl->var_decl.name, array);
    check(rc == 0, "Unable to find variable references: '%s'", bdata(var_decl->var_decl.name));
    
    // Bind all references to the variable.
    uint32_t i;
    for(i=0; i<array->length; i++) {
        qip_ast_node *var_ref = array->elements[i];
        rc = qip_ast_function_bind_apply(node, module, var_ref);
        check(rc == 0, "Unable to apply binding to function reference");
    }
    
    // Remove original variable declaration.
    rc = qip_ast_block_remove_expr(block, var_decl);
    check(rc == 0, "Unable to remove variable declaration from block");
    
    // Clean up.
    qip_array_free(array);
    
    return 0;

error:
    qip_array_free(array);
    return -1;
}

// Binds a function to a target based on the signature of the target. This
// currently only works when referencing unbound functions in a function call.
// The function argument acts as a placeholder and will be replaced by a bound
// version of the function.
//
// node        - The unbound function node.
// module      - The module that the node is a part of.
// target_node - The node that is referencing the unbound function.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_bind_apply(qip_ast_node *node, qip_module *module,
                                qip_ast_node *target_node)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(!node->function.bound, "Cannot apply binding to bound function");
    check(module != NULL, "Module required");
    check(target_node != NULL, "Target node required");
    
    // Copy the unbound function.
    qip_ast_node *bound_node = NULL;
    rc = qip_ast_node_copy(node, &bound_node);
    check(rc == 0, "Unable to copy unbound function");
    
    // It's not bound yet.
    bound_node->function.bound = false;
    
    // Replace the function argument with the bound function.
    if(target_node->parent->type == QIP_AST_TYPE_VAR_REF && target_node->parent->var_ref.type == QIP_AST_VAR_REF_TYPE_INVOKE) {
        unsigned int i;
        int32_t index = -1;
        for(i=0; i<target_node->parent->var_ref.arg_count; i++) {
            qip_ast_node *arg = target_node->parent->var_ref.args[i];
            if(arg == target_node) {
                index = (int32_t)i;
                break;
            }
        }
        
        // Raise error if target node is not part of the function call args.
        if(index == -1) {
            sentinel("Target node is not in parent call argument list");
        }
        
        // Retrieve original function AST node that is being invoked.
        qip_ast_node *function = NULL;
        rc = qip_ast_var_ref_get_invoke_function(target_node->parent, module, &function);
        check(rc == 0, "Unable to retrieve calling function");
        
        // Offset the index by one if this is a method.
        uint32_t arg_index = index + (function->parent && function->parent->type == QIP_AST_TYPE_METHOD ? 1 : 0);

        // Apply function argument type.
        qip_ast_node *arg_type_ref = function->function.args[arg_index]->farg.var_decl->var_decl.type;
        rc = qip_ast_function_bind_with_type_ref(bound_node, module, arg_type_ref);
        check(rc == 0, "Unable to bind with type ref");
        
        // Swap out argument with bound function.
        qip_ast_node_free(target_node->parent->var_ref.args[index]);
        target_node->parent->var_ref.args[index] = bound_node;
        bound_node->parent = target_node->parent;
        
        // Mark as bound.
        bound_node->function.bound = true;
    }
    else {
        sentinel("Invalid use of an unbound function");
    }
    
    return 0;

error:
    qip_ast_node_free(bound_node);
    return -1;
}

// Generates function arguments and returns type for an unbound function based
// on a function type ref.
//
// node     - The unbound function node.
// module   - The module.
// type_ref - The type ref to bind to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_function_bind_with_type_ref(qip_ast_node *node, qip_module *module,
                                        qip_ast_node *type_ref)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_FUNCTION, "Node type must be 'function'");
    check(node->function.arg_count == 0, "Function cannot have existing arguments");
    check(node->function.return_type == NULL, "Function cannot have an existing return type");
    check(module != NULL, "Module required");
    check(type_ref != NULL, "Type ref required");
    check(qip_is_function_type(type_ref), "Type ref must be a 'Function' type");
    
    // Generate function arguments.
    unsigned int i;
    qip_ast_node *arg_type_ref = NULL;
    qip_ast_node *farg = NULL;
    qip_ast_node *var_decl = NULL;
    for(i=0; i<type_ref->type_ref.subtype_count; i++) {
        qip_ast_node *subtype = type_ref->type_ref.subtypes[i];
        check(subtype->type_ref.arg_name != NULL, "Subtype argument name required");
        
        // Copy subtype.
        arg_type_ref = NULL;
        rc = qip_ast_type_ref_copy(subtype, &arg_type_ref);
        check(rc == 0, "Unable to copy subtype to argument type");
        
        // Clear argument name.
        bdestroy(arg_type_ref->type_ref.arg_name);
        arg_type_ref->type_ref.arg_name = NULL;
        
        // Create function argument.
        var_decl = qip_ast_var_decl_create(subtype, subtype->type_ref.arg_name, NULL);
        check_mem(var_decl);
        farg = qip_ast_farg_create(var_decl); check_mem(farg);
        
        // Add argument.
        rc = qip_ast_function_add_arg(node, farg);
        check(rc == 0, "Unable to add function argument");
    }

    // If we have a return type then apply it.
    qip_ast_node *return_type = NULL;
    if(type_ref->type_ref.return_type != NULL) {
        rc = qip_ast_type_ref_copy(type_ref->type_ref.return_type, &return_type);
        check(rc == 0, "Unable to copy return type");
    }
    // Otherwise set the return type to 'void'.
    else {
        return_type = qip_ast_type_ref_create_cstr("void");
        check_mem(return_type);
    }
    node->function.return_type = return_type;
    return_type->parent = node;
    
    return 0;

error:
    qip_ast_node_free(farg);
    qip_ast_node_free(return_type);
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
int qip_ast_function_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Append dump.
    bstring str = bformat("<function name='%s'>\n", bdatae(node->function.name, ""));
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump children
    if(node->function.return_type != NULL) {
        rc = qip_ast_node_dump(node->function.return_type, ret);
        check(rc == 0, "Unable to dump function return type");
    }

    unsigned int i;
    for(i=0; i<node->function.arg_count; i++) {
        rc = qip_ast_node_dump(node->function.args[i], ret);
        check(rc == 0, "Unable to dump function argument");
    }
    if(node->function.body != NULL) {
        rc = qip_ast_node_dump(node->function.body, ret);
        check(rc == 0, "Unable to dump function body");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
