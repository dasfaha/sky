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
    check(rc == 1, "Unable to write query text");

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

// Runs a PEACH query against a table.
//
// message - The message.
// table   - The table to run the query against.
//
// Returns 0 if successful, otherwise returns -1.
int sky_peach_message_process(sky_peach_message *message, sky_table *table)
{
    int rc;
    check(message != NULL, "Message required");
    check(table != NULL, "Table required");

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
    qip_compiler_add_class_path(compiler, &core_class_path);
    qip_compiler_add_class_path(compiler, &sky_class_path);
    rc = qip_compiler_compile(compiler, &module_name, message->query, args, 2, &module);
    check(rc == 0, "Unable to compile");
    if(module->error_count > 0) {
        debug("Parse error [line %d] %s", module->errors[0]->line_no, bdata(module->errors[0]->message));
    }
    check(module->error_count == 0, "Parse errors found");
    qip_compiler_free(compiler);

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
    debug("Paths processed: %d", path_count);

    // Retrieve Result serialization function.
    struct tagbstring result_str = bsStatic("Result");
    struct tagbstring serialize_str = bsStatic("serialize");
    qip_result_serialize_func result_serialize = NULL;
    qip_module_get_class_method(module, &result_str, &serialize_str, (void*)(&result_serialize));
    check(result_serialize != NULL, "Unable to find serialize() method on class 'Result'");

    // Serialize.
    qip_serializer *serializer = qip_serializer_create();
    qip_serializer_pack_map(serializer, map->count);
    int64_t i;
    for(i=0; i<map->count; i++) {
        result_serialize(map->elements[i], serializer);
    }

    // Send response to socket.
    //rc = write(socket, serializer->data, serializer->length);
    //check((int64_t)rc == serializer->length, "Unable to write serialized data to socket");
    
    qip_map_free(map);
    return 0;

error:
    qip_map_free(map);
    return -1;
}