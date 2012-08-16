#ifndef _eql_llvm_h
#define _eql_llvm_h

#include <stdbool.h>

#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include "bstring.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Types
//======================================

bool eql_llvm_is_pointer_type(LLVMTypeRef type);

bool eql_llvm_is_double_pointer_type(LLVMTypeRef type);

bool eql_llvm_is_complex_type(LLVMTypeRef type);


#endif
