#include <stdlib.h>
#include <stdbool.h>
#include "dbg.h"

#include "node.h"

//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an AST node for a literal array.
//
// Returns a literal boolean node.
qip_ast_node *qip_ast_array_literal_create()
{
    qip_ast_node *node = calloc(1, sizeof(qip_ast_node)); check_mem(node);
    node->type = QIP_AST_TYPE_ARRAY_LITERAL;
    return node;

error:
    qip_ast_node_free(node);
    return NULL;
}

// Frees the node.
//
// node - The node.
//
// Returns nothing.
void qip_ast_array_literal_free(qip_ast_node *node)
{
    if(node != NULL) {
        qip_ast_array_literal_free_items(node);
    }
}

// Frees the array items.
//
// node - The node.
//
// Returns nothing.
void qip_ast_array_literal_free_items(qip_ast_node *node)
{
    if(node != NULL) {
        unsigned int i;
        for(i=0; i<node->array_literal.item_count; i++) {
            qip_ast_node_free(node->array_literal.items[i]);
            node->array_literal.items[i] = NULL;
        }
        if(node->array_literal.items) free(node->array_literal.items);
        node->array_literal.items = NULL;
        node->array_literal.item_count = 0;

        // Free the type as well since it's generated based on the items.
        qip_ast_node_free(node->array_literal.type_ref);
    }
}

// Copies a node and its children.
//
// node - The node to copy.
// ret  - A pointer to where the new copy should be returned to.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_array_literal_copy(qip_ast_node *node, qip_ast_node **ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "Return pointer required");

    qip_ast_node *item_clone = NULL;
    qip_ast_node *clone = qip_ast_array_literal_create(); check_mem(clone);
    
    // Clone children.
    unsigned int i;
    for(i=0; i<node->array_literal.item_count; i++) {
        rc = qip_ast_node_copy(node->array_literal.items[i], &item_clone);
        check(rc == 0, "Unable to clone item");
        
        rc = qip_ast_array_literal_add_item(clone, item_clone);
        check(rc == 0, "Unable to add cloned item");
    }
    
    *ret = clone;
    return 0;

error:
    qip_ast_node_free(item_clone);
    qip_ast_node_free(clone);
    *ret = NULL;
    return -1;
}


//--------------------------------------
// Item Management
//--------------------------------------

// Adds an item to the array.
//
// node - The node.
// item - The item to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_array_literal_add_item(qip_ast_node *node, qip_ast_node *item)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_ARRAY_LITERAL, "Node type must be 'array literal'");
    check(item != NULL, "Item required");

    // Append item to array.
    node->array_literal.item_count++;
    node->array_literal.items = realloc(node->array_literal.items, sizeof(*node->array_literal.items) * node->array_literal.item_count);
    check_mem(node->array_literal.items);
    node->array_literal.items[node->array_literal.item_count-1] = item;
    
    // Link item to node.
    item->parent = node;

    return 0;

error:
    return -1;
}

// Adds a list of items to the array literal.
//
// node       - The node.
// items      - An array of items to add.
// item_count - The number of items to add.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_array_literal_add_items(qip_ast_node *node, qip_ast_node **items,
                                    unsigned int item_count)
{
    int rc;
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_ARRAY_LITERAL, "Node type must be 'array literal'");
    check(items != NULL || item_count == 0, "Items required");
    
    unsigned int i;
    for(i=0; i<item_count; i++) {
        rc = qip_ast_array_literal_add_item(node, items[i]);
        check(rc == 0, "Unable to add item");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the node.
//
// node    - The node.
// module  - The module.
// value   - A pointer to where the LLVM value should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_array_literal_codegen(qip_ast_node *node, qip_module *module,
                                  LLVMValueRef *value)
{
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");
    check(value != NULL, "Return pointer required");
    
    // Array literals are preprocessed into var_decl/var_refs.
    sentinel("Illegal codegen of an array literal");

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
int qip_ast_array_literal_preprocess(qip_ast_node *node, qip_module *module,
                                     qip_ast_processing_stage_e stage)
{
    int rc;
    bstring name = NULL;
    check(node != NULL, "Node required");
    check(module != NULL, "Module required");

    struct tagbstring elements_str = bsStatic("elements");
    struct tagbstring length_str = bsStatic("length");
    struct tagbstring elemsz_str = bsStatic("elemsz");
    struct tagbstring set_item_at_str = bsStatic("setItemAt");

    // Replace the array literal with a variable declaration and assignments
    // of each element. This is to allow arrays to be initialized just like
    // any other object.
    if(stage == QIP_AST_PROCESSING_STAGE_LOADING) {
        // Find index in block.
        qip_ast_node *block = NULL;
        int32_t expr_index = 0;
        rc = qip_ast_node_get_block_expr_index(node, &block, &expr_index);
        check(rc == 0 && block != NULL && expr_index != -1, "Unable to determine expression index in block");
        
        // Autogenerate non-conflicting variable name.
        name = bformat("$array%d", module->sequence++); check_mem(name);
        
        // Replace array literal with reference to array.
        qip_ast_node *var_ref = qip_ast_var_ref_create_value(name); check_mem(var_ref);
        rc = qip_ast_node_replace(node, var_ref);
        check(rc == 0, "Unable to replace array literal with variable reference");
        
        // Retrieve first element's type.
        qip_ast_node *element_type = NULL;
        rc = qip_ast_node_get_type(node->array_literal.items[0], module, &element_type);
        check(rc == 0, "Unable to determine array element type");
        
        // Create an array type based on initial element.
        qip_ast_node *subtype = NULL;
        qip_ast_node *type_ref = qip_ast_type_ref_create_cstr("Array"); check_mem(type_ref);
        rc = qip_ast_node_copy(element_type, &subtype);
        check(rc == 0, "Unable to copy element type");
        rc = qip_ast_type_ref_add_subtype(type_ref, subtype);
        check(rc == 0, "Unable to add Array subtype");
        
        // Generate variable declaration:
        //
        //   Array<T> $array0;
        qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, name, NULL); check_mem(var_decl);
        rc = qip_ast_block_insert_expr(block, var_decl, expr_index++);
        check(rc == 0, "Unable to insert generated array variable declaration");
        
        // Initialize elements array using an alloca:
        //
        //   $array0.elements = alloca($array0.length * $array0.elemsz);
        qip_ast_node *elements_ref = qip_ast_var_ref_create_property_access(name, &elements_str); check_mem(elements_ref);
        qip_ast_node *length_ref = qip_ast_var_ref_create_property_access(name, &length_str); check_mem(length_ref);
        qip_ast_node *elemsz_ref = qip_ast_var_ref_create_property_access(name, &elemsz_str); check_mem(elemsz_ref);
        qip_ast_node *mul_expr_node = qip_ast_binary_expr_create(QIP_BINOP_MUL, length_ref, elemsz_ref); check_mem(mul_expr_node);
        qip_ast_node *alloca_node = qip_ast_alloca_create(mul_expr_node); check_mem(alloca_node);
        qip_ast_node *assign_node = qip_ast_var_assign_create(elements_ref, alloca_node); check_mem(assign_node);
        rc = qip_ast_block_insert_expr(block, assign_node, expr_index++);
        check(rc == 0, "Unable to insert generated array elements alloca");
        
        // Generate a setItemAt() method invocation for every element:
        //
        //   $array0.setItemAt(item, i);
        unsigned int i;
        for(i=0; i<node->array_literal.item_count; i++) {
            qip_ast_node *set_item_at_args[2];
            set_item_at_args[0] = node->array_literal.items[i];
            set_item_at_args[1] = qip_ast_int_literal_create(i);
            qip_ast_node *set_item_at_invoke_node = qip_ast_var_ref_create_method_invoke(name, &set_item_at_str, set_item_at_args, 2);
            check_mem(set_item_at_invoke_node);
            rc = qip_ast_block_insert_expr(block, set_item_at_invoke_node, expr_index++);
            check(rc == 0, "Unable to insert generated array elements assignment");
        }
        
        // Free array literal.
        node->array_literal.item_count = 0;
        qip_ast_node_free(node);
    }

    bdestroy(name);

    return 0;

error:
    bdestroy(name);
    return -1;   
}


//--------------------------------------
// Type
//--------------------------------------

// Returns the type name of the AST node.
//
// node - The node.
// type - A pointer to where the type should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int qip_ast_array_literal_get_type(qip_ast_node *node,
                                   qip_ast_node **type_ref)
{
    check(node != NULL, "Node required");
    check(node->type == QIP_AST_TYPE_ARRAY_LITERAL, "Node type must be 'array literal'");
    
    *type_ref = node->array_literal.type_ref;
    return 0;

error:
    *type_ref = NULL;
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
int qip_ast_array_literal_dump(qip_ast_node *node, bstring ret)
{
    int rc;
    check(node != NULL, "Node required");
    check(ret != NULL, "String required");
    
    bstring str = bfromcstr("<array-literal>\n"); check_mem(str);
    check(bconcat(ret, str) == BSTR_OK, "Unable to append dump");

    unsigned int i;
    for(i=0; i<node->array_literal.item_count; i++) {
        rc = qip_ast_node_dump(node->array_literal.items[i], ret);
        check(rc == 0, "Unable to dump array item");
    }

    return 0;

error:
    if(str != NULL) bdestroy(str);
    return -1;
}
