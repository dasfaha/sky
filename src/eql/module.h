#ifndef _eql_module_h
#define _eql_module_h

#include <inttypes.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct eql_module eql_module;

#include "compiler.h"
#include "ast.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

// Defines a scope within the module. These are added and removed from a stack
// via the push_scope() and pop_scope() functions.
typedef struct eql_module_scope {
    eql_ast_node *node;
    LLVMValueRef *var_values;
    eql_ast_node **var_decls;
    int32_t var_count;
} eql_module_scope;

struct eql_module {
    eql_compiler *compiler;
    LLVMModuleRef llvm_module;
    LLVMValueRef llvm_function;
    LLVMExecutionEngineRef llvm_engine;
    LLVMPassManagerRef llvm_pass_manager;
    eql_module_scope **scopes;
    int32_t scope_count;
    LLVMTypeRef *types;
    eql_ast_node **type_nodes;
    int32_t type_count;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_module *eql_module_create(bstring name, eql_compiler *compiler);

void eql_module_free(eql_module *module);


//--------------------------------------
// Types
//--------------------------------------

int eql_module_get_type_ref(eql_module *module, bstring name,
    eql_ast_node **node, LLVMTypeRef *type);

int eql_module_add_type_ref(eql_module *module, eql_ast_node *class,
    LLVMTypeRef type);

int eql_module_cast_value(eql_module *module, LLVMValueRef value,
    bstring from_type_name, bstring to_type_name, LLVMValueRef *ret);


//--------------------------------------
// Scope
//--------------------------------------

int eql_module_push_scope(eql_module *module, eql_ast_node *node);

int eql_module_pop_scope(eql_module *module, eql_ast_node *node);

int eql_module_get_variable(eql_module *module, bstring name,
    eql_ast_node **var_decl, LLVMValueRef *value);

int eql_module_add_variable(eql_module *module, eql_ast_node *var_decl,
    LLVMValueRef value);


//--------------------------------------
// Execution
//--------------------------------------

int eql_module_get_main_function(eql_module *module, void **ret);

int eql_module_execute_int(eql_module *module, int64_t *ret);

int eql_module_execute_float(eql_module *module, double *ret);


//--------------------------------------
// Debugging
//--------------------------------------

int eql_module_dump(eql_module *module);

int eql_module_dump_to_file(eql_module *module, bstring filename);

#endif
