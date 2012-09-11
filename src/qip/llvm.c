#include <stdlib.h>

#include "llvm.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Types
//--------------------------------------

// Retrieves a flag stating if the LLVM type passed in is a pointer.
//
// type - The LLVM type to check.
//
// Returns true if the type is a pointer, otherwise returns false.
bool qip_llvm_is_pointer_type(LLVMTypeRef type)
{
    LLVMTypeKind type_kind = LLVMGetTypeKind(type);
    return (type_kind == LLVMPointerTypeKind);
}

// Retrieves a flag stating if the LLVM type passed in is a pointer to a pointer.
//
// type - The LLVM type to check.
//
// Returns true if the type is a pointer to a pointer, otherwise returns false.
bool qip_llvm_is_double_pointer_type(LLVMTypeRef type)
{
    if(qip_llvm_is_pointer_type(type)) {
        LLVMTypeRef element_type = LLVMGetElementType(type);
        return qip_llvm_is_pointer_type(element_type);
    }
    else {
        return false;
    }
}


// Retrieves a flag stating if the LLVM type passed in is a complex type
// (struct, array).
//
// type - The LLVM type to check.
//
// Returns true if the type is complex, otherwise returns false.
bool qip_llvm_is_complex_type(LLVMTypeRef type)
{
    LLVMTypeKind type_kind = LLVMGetTypeKind(type);
    return (type_kind == LLVMStructTypeKind || type_kind == LLVMArrayTypeKind);
}

