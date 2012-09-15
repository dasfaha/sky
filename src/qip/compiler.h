#ifndef _qip_compiler_h
#define _qip_compiler_h

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include "bstring.h"
#include "error.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

typedef struct qip_compiler qip_compiler;

#include "module.h"
#include "node.h"

typedef int (*qip_load_module_source_t)(qip_compiler *compiler, bstring name, bstring *source);
typedef int (*qip_process_dynamic_class_t)(qip_module *module, qip_ast_node *class);


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct qip_compiler {
    LLVMBuilderRef llvm_builder;
    bstring *class_paths;
    uint32_t class_path_count;
    bstring *dependencies;
    uint32_t dependency_count;
    qip_load_module_source_t load_module_source;
    qip_process_dynamic_class_t process_dynamic_class;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

qip_compiler *qip_compiler_create();

void qip_compiler_free(qip_compiler *compiler);


//======================================
// Compile
//======================================

int qip_compiler_compile(qip_compiler *compiler, qip_module *module, bstring source,
    qip_ast_node **args, uint32_t arg_count);


//======================================
// Module Management
//======================================

int qip_compiler_add_class_path(qip_compiler *compiler, bstring path);

int qip_compiler_load_module_source(qip_compiler *compiler, bstring name,
    bstring *source);


//======================================
// Dynamic Class Management
//======================================

int qip_compiler_process_dynamic_class(qip_compiler *compiler,
    qip_module *module, qip_ast_node *class);

#endif
