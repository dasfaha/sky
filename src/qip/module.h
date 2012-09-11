#ifndef _qip_module_h
#define _qip_module_h

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

typedef struct qip_module qip_module;

typedef void *(*sky_qip_function_t)();
typedef int64_t (*sky_qip_int_function_t)();
typedef double (*sky_qip_float_function_t)();
typedef bool (*sky_qip_boolean_function_t)();


#include "compiler.h"
#include "node.h"
#include "mempool.h"
#include "scope.h"


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct qip_module {
    qip_compiler *compiler;
    LLVMModuleRef llvm_module;
    LLVMExecutionEngineRef llvm_engine;
    LLVMPassManagerRef llvm_pass_manager;
    int64_t sequence;
    qip_ast_node **ast_modules;
    uint32_t ast_module_count;
    qip_scope **scopes;
    int32_t scope_count;
    LLVMTypeRef *types;
    qip_ast_node **type_nodes;
    int32_t type_count;
    qip_error **errors;
    uint32_t error_count;
    qip_mempool *perm_pool;
    qip_mempool *temp_pool;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

qip_module *qip_module_create(bstring name, qip_compiler *compiler);

void qip_module_free(qip_module *module);

void qip_module_free_errors(qip_module *module);

void qip_module_free_ast_modules(qip_module *module);

//--------------------------------------
// AST Module Management
//--------------------------------------

int qip_module_add_ast_module(qip_module *module, qip_ast_node *ast_module);

int qip_module_get_ast_class(qip_module *module, bstring name,
    qip_ast_node **ret);

int qip_module_get_ast_template_class(qip_module *module,
    qip_ast_node *type_ref, qip_ast_node **ret);

int qip_module_get_ast_main_function(qip_module *module, qip_ast_node **ret);

int qip_module_process_dynamic_classes(qip_module *module);

int qip_module_process_templates(qip_module *module);

int qip_module_generate_template_type(qip_module *module,
    qip_ast_node *type_ref, qip_ast_node **class);

int qip_module_preprocess(qip_module *module,
    qip_ast_processing_stage_e stage);

//--------------------------------------
// Types
//--------------------------------------

int qip_module_get_type_ref(qip_module *module, qip_ast_node *type_ref,
    qip_ast_node **node, LLVMTypeRef *type);

int qip_module_add_type_ref(qip_module *module, qip_ast_node *class,
    LLVMTypeRef type);

int qip_module_cast_value(qip_module *module, LLVMValueRef value,
    bstring from_type_name, bstring to_type_name, LLVMValueRef *ret);


//--------------------------------------
// Scope
//--------------------------------------

int qip_module_push_scope(qip_module *module, qip_scope *scope);

int qip_module_pop_scope(qip_module *module);

int qip_module_get_variable(qip_module *module, bstring name,
    qip_ast_node **var_decl, LLVMValueRef *value);

int qip_module_add_variable(qip_module *module, qip_ast_node *var_decl,
    LLVMValueRef value);

int qip_module_get_current_function_scope(qip_module *module,
    qip_scope **ret);


//--------------------------------------
// Execution
//--------------------------------------

int qip_module_get_main_function(qip_module *module, void **ret);

int qip_module_execute(qip_module *module, void **ret);

int qip_module_execute_int(qip_module *module, int64_t *ret);

int qip_module_execute_float(qip_module *module, double *ret);

int qip_module_execute_boolean(qip_module *module, bool *ret);

//--------------------------------------
// LLVM Function Management
//--------------------------------------

int qip_module_get_class_method(qip_module *module, bstring class_name,
    bstring method_name, void **ret);

//--------------------------------------
// Error Management
//--------------------------------------

int qip_module_add_error(qip_module *module, qip_ast_node *node,
    bstring message);

//--------------------------------------
// Memory Allocation
//--------------------------------------

int qip_module_perm_malloc(qip_module *module, size_t size, void **ptr);

int qip_module_temp_malloc(qip_module *module, size_t size, void **ptr);

int qip_module_reset_temp_pool(qip_module *module);

//--------------------------------------
// Debugging
//--------------------------------------

int qip_module_dump(qip_module *module);

int qip_module_dump_to_file(qip_module *module, bstring filename);

int qip_module_ast_dump_stderr(qip_module *module);

#endif
