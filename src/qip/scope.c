#include <stdlib.h>
#include <stdbool.h>

#include "scope.h"
#include "dbg.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a block scope.
qip_scope *qip_scope_create_block()
{
    qip_scope *scope = calloc(1, sizeof(qip_scope)); check_mem(scope);
    scope->type = QIP_SCOPE_TYPE_BLOCK;
    return scope;
    
error:
    qip_scope_free(scope);
    return NULL;
}

// Creates a function scope.
//
// llvm_function - The LLVM function value.
qip_scope *qip_scope_create_function(LLVMValueRef llvm_function)
{
    qip_scope *scope = calloc(1, sizeof(qip_scope)); check_mem(scope);
    scope->type = QIP_SCOPE_TYPE_FUNCTION;
    scope->llvm_function = llvm_function;
    return scope;
    
error:
    qip_scope_free(scope);
    return NULL;
}

// Frees a module.
//
// module - The module to free.
void qip_scope_free(qip_scope *scope)
{
    if(scope) {
        scope->llvm_function = NULL;
        qip_scope_free_vars(scope);
        free(scope);
    }
}

// Frees all variables on the scope.
//
// scope  - The scope.
//
// Returns nothing.
void qip_scope_free_vars(qip_scope *scope)
{
    if(scope != NULL) {
        free(scope->var_values);
        scope->var_values = NULL;
        free(scope->var_decls);
        scope->var_decls = NULL;
        scope->var_count = 0;
    }
}


//--------------------------------------
// Variable Management
//--------------------------------------

// Retrieves a variable with a given name declared within the scope.
//
// scope - The scope.
// name  - The name of the variable.
// node  - A pointer the AST node that created the value.
// value - A pointer to where the LLVM value should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_scope_get_variable(qip_scope *scope, bstring name,
                           qip_ast_node **node, LLVMValueRef *value)
{
    check(scope != NULL, "Scope is required");
    check(name != NULL, "Variable name is required");

    // Search for variables with a matching name.
    int32_t i;
    for(i=0; i<scope->var_count; i++) {
        if(biseq(scope->var_decls[i]->var_decl.name, name)) {
            if(node) *node   = scope->var_decls[i];
            if(value) *value = scope->var_values[i];
            return 0;
        }
    }

    // If we reach here then we couldn't find the variable in any scope.
    if(node) *node   = NULL;
    if(value) *value = NULL;
    return 0;

error:
    if(node) *node   = NULL;
    if(value) *value = NULL;
    return -1;
}

// Adds a variable declaration to scope.
//
// scope    - The scope.
// var_decl - The AST variable declaration to add.
// value    - The LLVM value associated with the declaration.
//
// Returns 0 if successful, otherwise returns -1.
int qip_scope_add_variable(qip_scope *scope, qip_ast_node *var_decl,
                           LLVMValueRef value)
{
    check(scope != NULL, "Scope is required");
    check(var_decl != NULL, "Variable declaration is required");
    check(var_decl->var_decl.name != NULL, "Variable declaration name is required");
    check(value != NULL, "LLVM value is required");

    // Search for existing variable in scope.
    int32_t i;
    for(i=0; i<scope->var_count; i++) {
        if(biseq(scope->var_decls[i]->var_decl.name, var_decl->var_decl.name)) {
            sentinel("Variable already exists in scope: %s", bdata(var_decl->var_decl.name));
        }
    }

    // Append variable declaration & LLVM value to scope.
    scope->var_count++;
    scope->var_decls = realloc(scope->var_decls, sizeof(qip_ast_node*) * scope->var_count);
    check_mem(scope->var_decls);
    scope->var_decls[scope->var_count-1] = var_decl;

    scope->var_values = realloc(scope->var_values, sizeof(LLVMValueRef) * scope->var_count);
    check_mem(scope->var_values);
    scope->var_values[scope->var_count-1] = value;

    return 0;

error:
    return -1;
}
