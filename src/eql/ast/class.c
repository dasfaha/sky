#include <stdlib.h>
#include <stdbool.h>
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

// Creates an AST node for a class.
//
// name           - The name of the class.
// methods        - An array of methods attached to the class.
// method_count   - The number of methods.
// properties     - An array of properties attached to the class.
// property_count - The number of properties.
// ret            - A pointer to where the ast node will be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_create(bstring name,
                         struct eql_ast_node **methods,
                         unsigned int method_count, 
                         struct eql_ast_node **properties,
                         unsigned int property_count, 
                         struct eql_ast_node **ret)
{
    int rc;
    
    eql_ast_node *node = malloc(sizeof(eql_ast_node)); check_mem(node);
    node->type = EQL_AST_TYPE_CLASS;
    node->parent = NULL;
    node->class.name = bstrcpy(name); check_mem(node->class.name);
    node->class.methods = NULL;
    node->class.method_count = 0;
    node->class.properties = NULL;
    node->class.property_count = 0;

    // Add methods.
    rc = eql_ast_class_add_members(node, methods, method_count);
    check(rc == 0, "Unable to add methods to class");

    // Add properties
    rc = eql_ast_class_add_members(node, properties, property_count);
    check(rc == 0, "Unable to add properties to class");

    // Clear metadata.
    node->class.metadata_count = 0;
    node->class.metadatas = NULL;
    
    *ret = node;
    return 0;

error:
    eql_ast_node_free(node);
    (*ret) = NULL;
    return -1;
}

// Frees a function call AST node from memory.
//
// node - The AST node to free.
void eql_ast_class_free(struct eql_ast_node *node)
{
    if(node->class.name) bdestroy(node->class.name);
    node->class.name = NULL;
    
    if(node->class.method_count > 0) {
        unsigned int i;
        for(i=0; i<node->class.method_count; i++) {
            eql_ast_node_free(node->class.methods[i]);
            node->class.methods[i] = NULL;
        }
        free(node->class.methods);
        node->class.method_count = 0;
    }

    if(node->class.property_count > 0) {
        unsigned int i;
        for(i=0; i<node->class.property_count; i++) {
            eql_ast_node_free(node->class.properties[i]);
            node->class.properties[i] = NULL;
        }
        free(node->class.properties);
        node->class.property_count = 0;
    }

    if(node->class.metadata_count > 0) {
        unsigned int i;
        for(i=0; i<node->class.metadata_count; i++) {
            eql_ast_node_free(node->class.metadatas[i]);
            node->class.metadatas[i] = NULL;
        }
        free(node->class.metadatas);
        node->class.metadata_count = 0;
    }
}


//--------------------------------------
// Member Management
//--------------------------------------

// Adds a property to a class.
//
// class    - The class to add the property to.
// property - The property to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_property(eql_ast_node *class,
                               eql_ast_node *property)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(property != NULL, "Property is required");
    
    // Append property to class.
    class->class.property_count++;
    class->class.properties = realloc(class->class.properties, sizeof(eql_ast_node*) * class->class.property_count);
    check_mem(class->class.properties);
    class->class.properties[class->class.property_count-1] = property;
    
    // Link property to class.
    property->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds a method to a class.
//
// class  - The class to add the method to.
// method - The method to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_method(eql_ast_node *class,
                             eql_ast_node *method)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(method != NULL, "Method is required");
    
    // Append method to class.
    class->class.method_count++;
    class->class.methods = realloc(class->class.methods, sizeof(eql_ast_node*) * class->class.method_count);
    check_mem(class->class.methods);
    class->class.methods[class->class.method_count-1] = method;
    
    // Link method to class.
    method->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds either a method or property to a class.
//
// class  - The class to add the member to.
// member - The method or property to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_member(eql_ast_node *class,
                             eql_ast_node *member)
{
    int rc;
    
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(member != NULL, "Member is required");

    // Add member via the appropriate function.
    if(member->type == EQL_AST_TYPE_PROPERTY) {
        rc = eql_ast_class_add_property(class, member);
        check(rc == 0, "Unable to add property to class");
    }
    else if(member->type == EQL_AST_TYPE_METHOD) {
        rc = eql_ast_class_add_method(class, member);
        check(rc == 0, "Unable to add method to class");
    }
    else {
        sentinel("Invalid class member type: %d", member->type);
    }
    
    return 0;

error:
    return -1;
}

// Adds a list of methods and properties to a class.
//
// class        - The class to add the member to.
// members      - A list of members to add.
// member_count - The number of members to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_members(eql_ast_node *class,
                              eql_ast_node **members,
                              unsigned int member_count)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(members != NULL || member_count == 0, "Members are required");

    // Add each member.
    unsigned int i;
    for(i=0; i<member_count; i++) {
        int rc = eql_ast_class_add_member(class, members[i]);
        check(rc == 0, "Unable to add member to class");
    }
    
    return 0;

error:
    return -1;
}



//--------------------------------------
// Metadata Management
//--------------------------------------

// Adds a metadata tag to a class.
//
// class    - The class to add the metadata to.
// metadata - The metadata to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_metadata(struct eql_ast_node *class,
                               struct eql_ast_node *metadata)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metadata != NULL, "Metadata is required");
    
    // Append metadata to class.
    class->class.metadata_count++;
    class->class.metadatas = realloc(class->class.metadatas, sizeof(eql_ast_node*) * class->class.metadata_count);
    check_mem(class->class.metadatas);
    class->class.metadatas[class->class.metadata_count-1] = metadata;
    
    // Link metadata to class.
    metadata->parent = class;
    
    return 0;

error:
    return -1;
}

// Adds a list of metatdata tags to a class.
//
// class           - The class to add the metatdata to.
// metatdatas      - A list of metatdatas to add.
// metatdata_count - The number of metatdatas to add.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_add_metadatas(eql_ast_node *class,
                                eql_ast_node **metatdatas,
                                unsigned int metatdata_count)
{
    // Validate.
    check(class != NULL, "Class is required");
    check(class->type == EQL_AST_TYPE_CLASS, "Class node is invalid type: %d", class->type);
    check(metatdatas != NULL || metatdata_count == 0, "Metadata tags are required");

    // Add each metatdata.
    unsigned int i;
    for(i=0; i<metatdata_count; i++) {
        int rc = eql_ast_class_add_metadata(class, metatdatas[i]);
        check(rc == 0, "Unable to add metatdata to class");
    }
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Codegen
//--------------------------------------

// Recursively generates LLVM code for the class AST node.
//
// node    - The node to generate an LLVM value for.
// module  - The compilation unit this node is a part of.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_codegen(eql_ast_node *node, eql_module *module)
{
	int rc;
    unsigned int i;

	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_CLASS, "Node type must be 'class'");
	check(module != NULL, "Module required");

	// Generate the methods of the class.
	unsigned int method_count = node->class.method_count;
	for(i=0; i<method_count; i++) {
		eql_ast_node *method = node->class.methods[i];
		LLVMValueRef func = NULL;
		rc = eql_ast_node_codegen(method, module, &func);
		check(rc == 0, "Unable to codegen method: %s", bdata(method->method.function->function.name));
	}
    
    return 0;

error:
    return -1;
}

// Generates and registers an LLVM struct for a given AST class. This struct
// can then be referenced within the module as the class' type.
//
// module - The compilation unit that contains the class.
// node   - The class AST node.
//
// Returns 0 if successful, otherwise returns -1.
int eql_ast_class_codegen_type(eql_module *module, eql_ast_node *node)
{
	int rc;
	unsigned int i;
	
	check(module != NULL, "Module required");
	check(node != NULL, "Node required");
	check(node->type == EQL_AST_TYPE_CLASS, "Node type must be 'class'");

    LLVMContextRef context = LLVMGetModuleContext(module->llvm_module);

	// Generate struct for class and properties.
	LLVMTypeRef llvm_struct = LLVMStructCreateNamed(context, bdata(node->class.name));

	// Generate the properties of the struct.
	unsigned int property_count = node->class.property_count;
	unsigned int element_count;
	
	// If there are no properties then generate a single property.
	LLVMTypeRef *elements;
	if(property_count == 0) {
		elements = malloc(sizeof(LLVMTypeRef));
        element_count = 1;
        bstring intTypeName = bfromcstr("Int");
		rc = eql_module_get_type_ref(module, intTypeName, NULL, &elements[0]);
        bdestroy(intTypeName);
		check(rc == 0, "Unable to retrieve Int type");
	}
	else {
        element_count = property_count;
		elements = malloc(sizeof(LLVMTypeRef) * property_count);
		for(i=0; i<property_count; i++) {
			eql_ast_node *property = node->class.properties[i];
			bstring property_type = property->property.var_decl->var_decl.type;
			rc = eql_module_get_type_ref(module, property_type, NULL, &elements[i]);
			check(rc == 0, "Unable to retrieve type: %s", bdata(property_type));
		}
	}

	// Set the struct body.
	LLVMStructSetBody(llvm_struct, elements, element_count, false);

	// Add type to the module.
	rc = eql_module_add_type_ref(module, node, llvm_struct);	
	check(rc == 0, "Unable to add type to module");
	
	return 0;
	
error:
	return -1;
}
