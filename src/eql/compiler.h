#ifndef _eql_compiler_h
#define _eql_compiler_h

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

typedef struct eql_compiler eql_compiler;

#include "module.h"
#include "node.h"

typedef int (*eql_load_module_source_t)(eql_compiler *compiler, bstring name, bstring *source);
typedef int (*eql_process_dynamic_class_t)(eql_compiler *compiler, eql_ast_node *class);


//==============================================================================
//
// Typedefs
//
//==============================================================================

struct eql_compiler {
    LLVMBuilderRef llvm_builder;
    bstring *class_paths;
    uint32_t class_path_count;
    eql_load_module_source_t load_module_source;
    eql_process_dynamic_class_t process_dynamic_class;
};


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

eql_compiler *eql_compiler_create();

void eql_compiler_free(eql_compiler *compiler);


//======================================
// Compile
//======================================

int eql_compiler_compile(eql_compiler *compiler, bstring name, bstring text,
    eql_ast_node **args, uint32_t arg_count, eql_module **ret);


//======================================
// Module Management
//======================================

int eql_compiler_add_class_path(eql_compiler *compiler, bstring path);

int eql_compiler_load_module_source(eql_compiler *compiler, bstring name,
    bstring *source);


//======================================
// Dynamic Class Management
//======================================

int eql_compiler_process_dynamic_class(eql_compiler *compiler, eql_ast_node *class);

#endif
