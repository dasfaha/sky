#include <stdlib.h>
#include <arpa/inet.h>

#include "peach_message.h"
#include "minipack.h"
#include "mem.h"
#include "dbg.h"

#include "qip/qip.h"
#include "qip_path.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef void (*sky_qip_path_map_func)(sky_qip_path *path, qip_map *map);
typedef void (*qip_result_serialize_func)(void *result, qip_serializer *serializer);

typedef int64_t (*sky_qip_get_dynamic_property_count_func)(void *event);
typedef void (*sky_qip_update_dynamic_property_arrays_func)(void *event, qip_fixed_array *ids, qip_fixed_array *offsets);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an PEACH message object.
//
// Returns a new PEACH message.
sky_peach_message *sky_peach_message_create()
{
    sky_peach_message *message = NULL;
    message = calloc(1, sizeof(sky_peach_message)); check_mem(message);
    return message;

error:
    sky_peach_message_free(message);
    return NULL;
}

// Frees an PEACH message object from memory.
//
// message - The message.
void sky_peach_message_free(sky_peach_message *message)
{
    if(message) {
        free(message);
    }
}


//--------------------------------------
// Serialization
//--------------------------------------

// Calculates the total number of bytes needed to store the message.
//
// message - The message.
//
// Returns the number of bytes required to store the message.
size_t sky_peach_message_sizeof(sky_peach_message *message)
{
    size_t sz = 0;
    sz += minipack_sizeof_raw(blength(message->query));
    sz += blength(message->query);
    return sz;
}

// Serializes an PEACH message to a memory location.
//
// message - The message.
// file    - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_peach_message_pack(sky_peach_message *message, FILE *file)
{
    int rc;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Database name
    rc = sky_minipack_fwrite_bstring(file, message->query);
    check(rc == 0, "Unable to write query text");

    return 0;

error:
    return -1;
}

// Deserializes an PEACH message from a file stream.
//
// message - The message.
// file    - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_peach_message_unpack(sky_peach_message *message, FILE *file)
{
    int rc;
    check(message != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Query
    rc = sky_minipack_fread_bstring(file, &message->query);
    check(rc == 0, "Unable to read query text");

    return 0;

error:
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Preprocesses dynamic classes to add database properties.
//
// compiler - The compiler.
// class    - The class AST node.
// data     - A pointer to the table.
//
// Returns 0 if successful, otherwise returns -1.
int sky_peach_message_process_dynamic_class_callback(qip_module *module, qip_ast_node *class, void *data)
{
    int rc;
    qip_array *array = NULL;
    check(module != NULL, "Module required");
    check(class != NULL, "Class required");
    check(data != NULL, "Table required");
    
    struct tagbstring this_str = bsStatic("this");
    struct tagbstring event_str = bsStatic("Event");
    struct tagbstring gdpc_str = bsStatic("getDynamicPropertyCount");
    struct tagbstring udpa_str = bsStatic("updateDynamicPropertyArrays");
    struct tagbstring ids_str = bsStatic("ids");
    struct tagbstring offsets_str = bsStatic("offsets");
    struct tagbstring set_item_at_str = bsStatic("setItemAt");

    // Convert data reference to a table.
    sky_table *table = (sky_table*)data;
    
    // Dynamically add properties from database to Event class if they are
    // referenced.
    if(biseqcstr(class->class.name, "Event")) {
        array = qip_array_create(); check_mem(array);
        
        // Retrieve getDynamicPropertyCount() method.
        qip_ast_node *gdpc_method = NULL;
        rc = qip_ast_class_get_method(class, &gdpc_str, &gdpc_method);
        check(rc == 0 && gdpc_method != NULL, "Unable to find Event.getDynamicPropertyCount method");
        qip_ast_node *gdpc_block = gdpc_method->method.function->function.body;
        qip_ast_block_free_exprs(gdpc_block);

        // Retrieve updateDynamicPropertyArrays() method.
        qip_ast_node *udpa_method = NULL;
        rc = qip_ast_class_get_method(class, &udpa_str, &udpa_method);
        check(rc == 0 && udpa_method != NULL, "Unable to find Event.updateDynamicPropertyArrays method");
        qip_ast_node *udpa_block = udpa_method->method.function->function.body;
        qip_ast_block_free_exprs(udpa_block);
        
        // Loop over each module to find Event variable references.
        uint32_t i;
        for(i=0; i<module->ast_module_count; i++) {
            qip_ast_node *ast_module = module->ast_modules[i];
            rc = qip_ast_node_get_var_refs_by_type(ast_module, module, &event_str, array);
            check(rc == 0, "Unable to search for Event variable references");
        }

        // Loop over references and add properties as needed.
        int64_t dynamic_property_count = 0;
        
        for(i=0; i<array->length; i++) {
            qip_ast_node *var_ref = (qip_ast_node*)array->elements[i];
            
            // Add property if member is a property reference.
            if(var_ref->var_ref.member && var_ref->var_ref.member->var_ref.type == QIP_AST_VAR_REF_TYPE_VALUE) {
                bstring property_name = var_ref->var_ref.member->var_ref.name;
                
                // Check if it exists on the Event class.
                qip_ast_node *property = NULL;
                rc = qip_ast_class_get_property(class, property_name, &property);
                check(rc == 0, "Unable to retrieve property from Event class");
                
                // Only add one if it doesn't exist.
                if(property == NULL) {
                    // Lookup property in the database.
                    sky_property *db_property = NULL;
                    rc = sky_property_file_find_by_name(table->property_file, property_name, &db_property);
                    check(rc == 0, "Unable to find property '%s' in table: %s", bdata(property_name), bdata(table->path));
                    
                    // Generate the property.
                    qip_ast_node *type_ref = qip_ast_type_ref_create(db_property->data_type);
                    qip_ast_node *var_decl = qip_ast_var_decl_create(type_ref, property_name, NULL);
                    property = qip_ast_property_create(QIP_ACCESS_PUBLIC, var_decl);

                    // Add it to the class.
                    rc = qip_ast_class_add_property(class, property);
                    check(rc == 0, "Unable to add property to class");
                    
                    // Add to db property id to updateDynamicPropertyArrays() method:
                    //
                    //   ids.setItemAt(<index>, <db-prop-id>);
                    qip_ast_node *args[2];
                    args[0] = qip_ast_int_literal_create(db_property->id);
                    args[1] = qip_ast_int_literal_create(dynamic_property_count);
                    rc = qip_ast_block_add_expr(udpa_block, qip_ast_var_ref_create_method_invoke(&ids_str, &set_item_at_str, args, 2));
                    check(rc == 0, "Unable to add property id to updateDynamicPropertyArrays()");

                    // Add to db property id to updateDynamicPropertyArrays() method:
                    //
                    //   offsets.setItemAt(offsetof(this.<db-prop-name>), <index>);
                    args[0] = qip_ast_offsetof_create(
                        qip_ast_var_ref_create_property_access(&this_str, property_name)
                    );
                    args[1] = qip_ast_int_literal_create(dynamic_property_count);
                    rc = qip_ast_block_add_expr(udpa_block, qip_ast_var_ref_create_method_invoke(&offsets_str, &set_item_at_str, args, 2));
                    check(rc == 0, "Unable to add property id to updateDynamicPropertyArrays()");

                    // Increment counter.
                    dynamic_property_count++;
                }
            }
        }
        
        // Append dynamic property count return.
        rc = qip_ast_block_add_expr(gdpc_block, 
            qip_ast_freturn_create(
                qip_ast_int_literal_create(dynamic_property_count)
            )
        );
        check(rc == 0, "Unable to add return to getDynamicPropertyCount() block");

        // Append void return for dynamic array method.
        rc = qip_ast_block_add_expr(udpa_block, qip_ast_freturn_create(NULL));
        check(rc == 0, "Unable to add return to updateDynamicPropertyArrays() block");
    }
    
    qip_array_free(array);
    return 0;

error:
    qip_array_free(array);
    return -1;
}

// Runs a PEACH query against a table.
//
// message - The message.
// table   - The table to run the query against.
// output  - The output stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_peach_message_process(sky_peach_message *message, sky_table *table,
                              FILE *output)
{
    int rc;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");
    check(output != NULL, "Output stream required");

    struct tagbstring event_str = bsStatic("Event");
    struct tagbstring gdpc_str = bsStatic("getDynamicPropertyCount");
    struct tagbstring udpa_str = bsStatic("updateDynamicPropertyArrays");

    // Compile query and execute.
    qip_ast_node *type_ref, *var_decl;
    qip_ast_node *args[2];
    
    // Path argument.
    struct tagbstring path_str = bsStatic("path");
    type_ref = qip_ast_type_ref_create_cstr("Path");
    var_decl = qip_ast_var_decl_create(type_ref, &path_str, NULL);
    args[0] = qip_ast_farg_create(var_decl);
    
    // Data argument.
    struct tagbstring data_str = bsStatic("data");
    type_ref = qip_ast_type_ref_create_cstr("Map");
    qip_ast_type_ref_add_subtype(type_ref, qip_ast_type_ref_create_cstr("Int"));
    qip_ast_type_ref_add_subtype(type_ref, qip_ast_type_ref_create_cstr("Result"));
    var_decl = qip_ast_var_decl_create(type_ref, &data_str, NULL);
    args[1] = qip_ast_farg_create(var_decl);

    // Compile.
    qip_module *module = NULL;
    struct tagbstring module_name = bsStatic("sky");
    struct tagbstring core_class_path = bsStatic("lib/core");
    struct tagbstring sky_class_path = bsStatic("lib/sky");
    qip_compiler *compiler = qip_compiler_create();
    compiler->process_dynamic_class = sky_peach_message_process_dynamic_class_callback;
    compiler->dependency_count = 1;
    compiler->dependencies = calloc(compiler->dependency_count, sizeof(*compiler->dependencies));
    compiler->dependencies[0] = bfromcstr("String");
    qip_compiler_add_class_path(compiler, &core_class_path);
    qip_compiler_add_class_path(compiler, &sky_class_path);
    rc = qip_compiler_compile(compiler, &module_name, message->query, args, 2, (void*)table, &module);
    check(rc == 0, "Unable to compile");
    if(module->error_count > 0) {
        debug("Parse error [line %d] %s", module->errors[0]->line_no, bdata(module->errors[0]->message));
    }
    check(module->error_count == 0, "Parse errors found");
    qip_compiler_free(compiler);

    // Retrieve Event functions for dynamic property info.
    sky_qip_get_dynamic_property_count_func get_dynamic_property_count = NULL;
    rc = qip_module_get_class_method(module, &event_str, &gdpc_str, (void*)&get_dynamic_property_count);
    check(rc == 0 && get_dynamic_property_count != NULL, "Unable to find Event.getDynamicPropertyCount() function");

    sky_qip_update_dynamic_property_arrays_func update_dynamic_property_arrays = NULL;
    rc = qip_module_get_class_method(module, &event_str, &udpa_str, (void*)&update_dynamic_property_arrays);
    check(rc == 0 && get_dynamic_property_count != NULL, "Unable to find Event.updateDynamicPropertyArrays() function");

    // Retrieve dynamic property info.
    int64_t dynamic_property_count = get_dynamic_property_count(NULL);
    qip_fixed_array *_ids = qip_fixed_array_create(sizeof(int64_t), dynamic_property_count);
    qip_fixed_array *_offsets = qip_fixed_array_create(sizeof(int64_t), dynamic_property_count);
    update_dynamic_property_arrays(NULL, _ids, _offsets);

    // Simplify arrays.
    int64_t *ids = (int64_t*)_ids->elements;
    int64_t *offsets = (int64_t*)_offsets->elements;
    _ids->elements = NULL;
    qip_fixed_array_free(_ids);
    _offsets->elements = NULL;
    qip_fixed_array_free(_offsets);

    // Print dynamic info.
    debug("dynamic property count: %lld", dynamic_property_count);
    int64_t j;
    for(j=0; j<dynamic_property_count; j++) {
        debug("  [%lld] id: %lld, offset: %lld", j, ids[j], offsets[j]);
    }

    //qip_module_dump(module);
    
    // Retrieve main function.
    sky_qip_path_map_func process_path = NULL;
    rc = qip_module_get_main_function(module, (void*)(&process_path));
    check(rc == 0, "Unable to retrieve main function");
 
    // Initialize the path iterator.
    sky_path_iterator iterator;
    sky_path_iterator_init(&iterator);
    rc = sky_path_iterator_set_data_file(&iterator, table->data_file);
    check(rc == 0, "Unable to initialze path iterator");

    // Initialize QIP args.
    sky_qip_path *path = sky_qip_path_create();
    qip_map *map = qip_map_create();
    
    // Iterate over each path.
    uint32_t path_count = 0;
    while(!iterator.eof) {
        // Retrieve the path pointer.
        rc = sky_path_iterator_get_ptr(&iterator, &path->path_ptr);
        check(rc == 0, "Unable to retrieve the path iterator pointer");
    
        // Execute query.
        process_path(path, map);

        // Move to next path.
        rc = sky_path_iterator_next(&iterator);
        check(rc == 0, "Unable to find next path");
        
        path_count++;
    }
    //debug("Paths processed: %d", path_count);

    // Retrieve Result serialization function.
    struct tagbstring result_str = bsStatic("Result");
    struct tagbstring serialize_str = bsStatic("serialize");
    qip_result_serialize_func result_serialize = NULL;
    qip_module_get_class_method(module, &result_str, &serialize_str, (void*)(&result_serialize));
    check(result_serialize != NULL, "Unable to find serialize() method on class 'Result'");

    // Serialize.
    qip_serializer *serializer = qip_serializer_create();
    qip_serializer_pack_map(module, serializer, map->count);
    int64_t i;
    for(i=0; i<map->count; i++) {
        result_serialize(map->elements[i], serializer);
    }

    // Send response to output stream.
    rc = fwrite(serializer->data, serializer->length, 1, output);
    check(rc == 1, "Unable to write serialized data to stream");
    
    qip_map_free(map);
    return 0;

error:
    qip_map_free(map);
    return -1;
}