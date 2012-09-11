#ifndef _qip_scope_h
#define _qip_scope_h

#include <stdbool.h>
#include <inttypes.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct qip_scope qip_scope;

#include "node.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

// Defines the types of scope.
typedef enum qip_scope_type_e {
    QIP_SCOPE_TYPE_BLOCK,
    QIP_SCOPE_TYPE_FUNCTION
} qip_scope_type_e;

// Defines a function or block scope within a module. If the llvm_function is 
// defined then this is a function scope. Otherwise it is a block scope.
struct qip_scope {
    qip_scope_type_e type;
    qip_ast_node *node;
    LLVMValueRef llvm_function;
    LLVMValueRef llvm_last_alloca;
    LLVMValueRef *var_values;
    qip_ast_node **var_decls;
    int32_t var_count;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_scope *qip_scope_create_block();

qip_scope *qip_scope_create_function(LLVMValueRef llvm_function);

void qip_scope_free(qip_scope *scope);

void qip_scope_free_vars(qip_scope *scope);

//--------------------------------------
// Variable Management
//--------------------------------------

int qip_scope_get_variable(qip_scope *scope, bstring name,
    qip_ast_node **var_decl, LLVMValueRef *value);

int qip_scope_add_variable(qip_scope *scope, qip_ast_node *var_decl,
    LLVMValueRef value);

#endif
