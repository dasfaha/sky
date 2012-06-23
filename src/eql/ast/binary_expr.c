#include <stdlib.h>
#include "../../dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a binary expression.
//
// operator - The operator used in the expression.
// lhs      - The node on the left-hand side.
// rhs      - The node on the right-hand side.
// ret      - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_binary_expr_create(eql_ast_binop_e operator,
                               eql_ast_node *lhs,
                               eql_ast_node *rhs,
                               eql_ast_node **ret)
{
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_BINARY_EXPR;
    node->parent = NULL;
    node->binary_expr.operator = operator;

    node->binary_expr.lhs = lhs;
    if(lhs != NULL) {
        lhs->parent = node;
    }

    node->binary_expr.rhs = rhs;
    if(rhs != NULL) {
        rhs->parent = node;
    }

    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a binary expression AST node from memory.
//
// node - The AST node to free.
void eql_ast_binary_expr_free(eql_ast_node *node)
{
    if(node->binary_expr.lhs) {
        eql_ast_node_free(node->binary_expr.lhs);
    }
    node->binary_expr.lhs = NULL;

    if(node->binary_expr.rhs) {
        eql_ast_node_free(node->binary_expr.rhs);
    }
    node->binary_expr.rhs = NULL;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the binary expression AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_binary_expr_codegen(eql_ast_node *node,
                                eql_module *module,
                                LLVMValueRef *value)
{
    int rc;
    LLVMValueRef lhs, rhs;
    
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BINARY_EXPR, "Node type must be 'binary expression'");
    check(module != NULL, "Module required");
    
    LLVMBuilderRef builder = module->compiler->llvm_builder;

    // Evaluate left and right hand values.
    rc = eql_ast_node_codegen(node->binary_expr.lhs, module, &lhs);
    check(rc == 0 && lhs != NULL, "Unable to codegen lhs");
    rc = eql_ast_node_codegen(node->binary_expr.rhs, module, &rhs);
    check(rc == 0 && rhs != NULL, "Unable to codegen rhs");

    // If values are different types then cast RHS to LHS.
    LLVMTypeRef lhs_type = LLVMTypeOf(lhs);
    LLVMTypeKind lhs_type_kind = LLVMGetTypeKind(lhs_type);
    LLVMTypeRef rhs_type = LLVMTypeOf(rhs);
    LLVMTypeKind rhs_type_kind = LLVMGetTypeKind(rhs_type);

    if(lhs_type != rhs_type) {
        // Cast int to float.
        if(lhs_type_kind == LLVMDoubleTypeKind && rhs_type_kind == LLVMIntegerTypeKind)
        {
            rhs = LLVMBuildSIToFP(builder, rhs, lhs_type, "");
        }
        // Cast float to int.
        else if(lhs_type_kind == LLVMIntegerTypeKind && rhs_type_kind == LLVMDoubleTypeKind)
        {
            rhs = LLVMBuildFPToSI(builder, rhs, lhs_type, "");
        }
        // Throw error if it's any other conversion.
        else {
            sentinel("Unable to cast types");
        }
    }

    // Generate Float operations.
    if(lhs_type_kind == LLVMDoubleTypeKind) {
        switch(node->binary_expr.operator) {
            case EQL_BINOP_PLUS: {
                *value = LLVMBuildFAdd(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_MINUS: {
                *value = LLVMBuildFSub(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_MUL: {
                *value = LLVMBuildFMul(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_DIV: {
                *value = LLVMBuildFDiv(builder, lhs, rhs, "");
                break;
            }
			default: {
				sentinel("Invalid float binary operator");
			}
        }
    }
    // Generate Integer operations.
    else {
        switch(node->binary_expr.operator) {
            case EQL_BINOP_PLUS: {
                *value = LLVMBuildAdd(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_MINUS: {
                *value = LLVMBuildSub(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_MUL: {
                *value = LLVMBuildMul(builder, lhs, rhs, "");
                break;
            }
            case EQL_BINOP_DIV: {
                *value = LLVMBuildSDiv(builder, lhs, rhs, "");
                break;
            }
			default: {
				sentinel("Invalid int binary operator");
			}
        }
    }
	
    check(*value != NULL, "Unable to codegen binary expression");
    
    return 0;

error:
    *value = NULL;
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The AST node to determine the type for.
// type - A pointer to where the type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_binary_expr_get_type(eql_ast_node *node, bstring *type)
{
    check(node != NULL, "Node required");
    check(node->type == EQL_AST_TYPE_BINARY_EXPR, "Node type must be 'binary expr'");
    check(node->binary_expr.lhs != NULL, "Binary expression LHS is required");
    
    // Return the type of the left side.
    int rc = eql_ast_node_get_type(node->binary_expr.lhs, type);
    check(rc == 0, "Unable to determine the binary expression type");
    
    return 0;

error:
    *type = NULL;
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
int eql_ast_binary_expr_dump(eql_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");

    // Determine operator symbol.
    char *operator = "";
    switch(node->binary_expr.operator) {
        case EQL_BINOP_PLUS: operator = "+"; break;
        case EQL_BINOP_MINUS: operator = "-"; break;
        case EQL_BINOP_MUL: operator = "*"; break;
        case EQL_BINOP_DIV: operator = "/"; break;
    }
    
    // Append dump.
    bstring str = bformat("<binary-expr operator='%s'>\n", operator);
    check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    // Recursively dump LHS & RHS.
    if(node->binary_expr.lhs != NULL) {
        rc = eql_ast_node_dump(node->binary_expr.lhs, ret);
        check(rc == 0, "Unable to dump LHS");
    }
    if(node->binary_expr.rhs != NULL) {
        rc = eql_ast_node_dump(node->binary_expr.rhs, ret);
        check(rc == 0, "Unable to dump RHS");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
