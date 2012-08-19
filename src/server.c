#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "bstring.h"
#include "server.h"
#include "message.h"
#include "qip/qip.h"
#include "qip_path.h"
#include "dbg.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

typedef void (*sky_qip_path_map_func)(sky_qip_path *path, qip_map *map);


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_server_open_table(sky_server *server, bstring database_name,
    bstring table_name, sky_database **database, sky_table **table);

int sky_server_close_table(sky_server *server, sky_database *database,
    sky_table *table);


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

// Creates a reference to a server instance.
//
// path - The directory path where the databases reside.
//
// Returns a reference to the server.
sky_server *sky_server_create(bstring path)
{
    sky_server *server = NULL;
    server = calloc(1, sizeof(sky_server)); check_mem(server);
    
    server->path = path;
    server->port = SKY_DEFAULT_PORT;
    
    return server;

error:
    sky_server_free(server);
    return NULL;
}

// Frees a server instance from memory.
//
// server - The server object to free.
void sky_server_free(sky_server *server)
{
    if(server) {
        if(server->path) bdestroy(server->path);
        free(server);
    }
}


//======================================
// State
//======================================

// Starts a server. Once a server is started, it can accept messages over TCP
// on the bind address and port number specified by the server object.
//
// server - The server to start.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_start(sky_server *server)
{
    int rc;

    check(server != NULL, "Server required");
    check(server->state == SKY_SERVER_STATE_STOPPED, "Server already running");
    check(server->port > 0, "Port required");

    // Initialize socket info.
    server->sockaddr = calloc(1, sizeof(struct sockaddr_in));
    check_mem(server->sockaddr);
    server->sockaddr->sin_addr.s_addr = INADDR_ANY;
    server->sockaddr->sin_port = htons(server->port);
    server->sockaddr->sin_family = AF_INET;

    // Create socket.
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    check(server->socket != -1, "Unable to create a socket");
    
    // Bind socket.
    rc = bind(server->socket, (struct sockaddr*)server->sockaddr, sizeof(struct sockaddr_in));
    check(rc == 0, "Unable to bind socket");
    
    // Listen on socket.
    rc = listen(server->socket, SKY_LISTEN_BACKLOG);
    check(rc != -1, "Unable to listen on socket");
    
    // Update server state.
    server->state = SKY_SERVER_STATE_RUNNING;
    
    return 0;

error:
    sky_server_stop(server);
    return -1;
}

// Stops a server. This actions closes the TCP socket and in-process messages
// will be aborted.
//
// server - The server to stop.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_stop(sky_server *server)
{
    // Close socket if open.
    if(server->socket > 0) {
        close(server->socket);
    }
    server->socket = 0;

    // Clear socket info.
    if(server->sockaddr) {
        free(server->sockaddr);
    }
    server->sockaddr = NULL;
    
    // Update server state.
    server->state = SKY_SERVER_STATE_STOPPED;
    
    return 0;
}


//======================================
// Connection Management
//======================================

// Accepts a connection on a running server. Once a connection is accepted then
// the message is parsed and processed.
//
// server - The server to start.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_accept(sky_server *server)
{
    int rc;
    void *buffer;
    
    // Accept the next connection.
    int sockaddr_size = sizeof(struct sockaddr_in);
    int socket = accept(server->socket, (struct sockaddr*)server->sockaddr, (socklen_t *)&sockaddr_size);
    check(socket != -1, "Unable to accept connection");
    
    // Read message header.
    buffer = calloc(1, SKY_MESSAGE_HEADER_LENGTH);
    rc = read(socket, buffer, SKY_MESSAGE_HEADER_LENGTH);
    check(rc == SKY_MESSAGE_HEADER_LENGTH, "Unable to read message header");
    
    // Parse message header.
    sky_message_header *header = sky_message_header_create();
    check_mem(header);
    rc = sky_message_header_parse(buffer, header);
    check(rc == 0, "Unable to parse message header");
    
    // Extend buffer for length of message.
    uint32_t buffer_length = SKY_MESSAGE_HEADER_LENGTH + header->length;
    buffer = realloc(buffer, buffer_length);
    check_mem(buffer);

    // Read the message body.
    rc = read(socket, buffer+SKY_MESSAGE_HEADER_LENGTH, header->length);
    check((uint32_t)rc == header->length, "Unable to read message body");

    // Parse appropriate message type.
    switch(header->type) {
        case SKY_MESSAGE_EADD: {
            rc = sky_server_process_eadd_message(server, socket, buffer);
            check(rc == 0, "Unable to process EADD message");
            break;
        }
        
        case SKY_MESSAGE_PEACH: {
            rc = sky_server_process_peach_message(server, socket, buffer);
            check(rc == 0, "Unable to process PEACH message");
            break;
        }
        
        default: {
            sentinel("Invalid message type");
            break;
        }
    }
    
    // Clean up.
    if(header) sky_message_header_free(header);
    close(socket);

    return 0;

error:
    if(header) sky_message_header_free(header);
    if(socket > 0) close(socket);
    return -1;
}


//======================================
// EADD
//======================================

// Processes an "Event Add" message.
//
// server - The server that received the message.
// socket - The socket that sent the message.
// buffer - The buffer that contains the full message.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_process_eadd_message(sky_server *server, int socket,
                                    void *buffer)
{
    int rc;
    check(server != NULL, "Server required");
    check(socket != 0, "Socket required");
    check(buffer != NULL, "Buffer required");
    
    // Parse message from buffer.
    sky_eadd_message *message = sky_eadd_message_create();
    rc = sky_eadd_message_parse(buffer, message);
    
    // Validate message.
    check(server->path != NULL, "Server path is required");
    check(message->database_name != NULL, "Database name is required");
    check(message->table_name != NULL, "Table name is required");
    check(message->object_id != 0, "Object ID is required");
    
    // Open table.
    sky_database *database = NULL;
    sky_table *table = NULL;
    sky_server_open_table(server, message->database_name, message->table_name, &database, &table);

    // Create event.
    sky_event *event = sky_event_create(message->object_id, message->timestamp, message->action_id);
    
    // Add event to table.
    rc = sky_table_add_event(table, event);
    check(rc == 0, "Unable to add event to table");
    
    // TODO: Send respond to socket.
    
    // Close table.
    sky_server_close_table(server, database, table);

    // Clean up.
    sky_event_free(event);
    sky_eadd_message_free(message);

    return 0;

error:
    sky_event_free(event);
    sky_eadd_message_free(message);
    return -1;
}


//======================================
// PEACH
//======================================

// Processes a "Each Path" message.
//
// server - The server.
// socket - The socket that sent the message.
// buffer - The buffer that contains the full message.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_process_peach_message(sky_server *server, int socket,
                                     void *buffer)
{
    int rc;
    check(server != NULL, "Server required");
    check(socket != 0, "Socket required");
    check(buffer != NULL, "Buffer required");
    
    // Parse message from buffer.
    sky_peach_message *message = sky_peach_message_create();
    rc = sky_peach_message_parse(buffer, message);
    
    // Validate message.
    check(message->database_name != NULL, "Database name is required");
    check(message->table_name != NULL, "Table name is required");
    check(message->query != NULL, "Query text is required");
    
    // Open table.
    sky_database *database = NULL;
    sky_table *table = NULL;
    sky_server_open_table(server, message->database_name, message->table_name, &database, &table);

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

    // TODO: Create serializer.
    // TODO: Send response to socket.
    
    // Close table.
    sky_server_close_table(server, database, table);

    // Clean up.
    qip_map_free(map);
    qip_module_free(module);
    sky_peach_message_free(message);

    return 0;

error:
    qip_map_free(map);
    qip_module_free(module);
    sky_peach_message_free(message);
    return -1;
}


//======================================
// Table management
//======================================

// Opens an table.
//
// server - The server that is opening the table.
// database_name - The name of the database to open.
// table_name - The name of the table to open.
// database - Returns the instance of the database to the caller. 
// table - Returns the instance of the table to the caller. 
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_open_table(sky_server *server, bstring database_name,
                          bstring table_name, sky_database **database,
                          sky_table **table)
{
    int rc;
    check(server != NULL, "Server required");
    check(database_name != NULL, "Database name required");
    check(table_name != NULL, "Table name required");
    
    // Initialize return values.
    *database = NULL;
    *table    = NULL;
    
    // Determine the path to the database.
    bstring database_path = bformat("%s/%s", bdata(server->path), bdata(database_name)); 
    check_mem(database_path);
    
    // Create the database.
    *database = sky_database_create(); check_mem(*database);
    rc = sky_database_set_path(*database, database_path);
    check(rc == 0, "Unable to set database path");
    
    // Determine the path to the table.
    bstring table_path = bformat("%s/%s", bdata(database_path), bdata(table_name));
    check_mem(table_path);

    // Create the table.
    *table = sky_table_create(); check_mem(*table);
    rc = sky_table_set_path(*table, table_path);
    check(rc == 0, "Unable to set table path");
    
    // Open the table.
    rc = sky_table_open(*table);
    check(rc == 0, "Unable to open table");
    
    return 0;

error:
    sky_database_free(*database);
    sky_table_free(*table);
    *database = NULL;
    *table    = NULL;
    return -1;
}

// Closes an table.
//
// server - The server that is opening the table.
// database - The database that the table belongs to.
// table - The table to close.
//
// Returns 0 if successful, otherwise returns -1.
int sky_server_close_table(sky_server *server, sky_database *database,
                           sky_table *table)
{
    int rc;
    check(server != NULL, "Server required");
    check(database != NULL, "Database required");
    check(database != NULL, "Table required");
    
    // Close the table.
    rc = sky_table_close(table);
    check(rc == 0, "Unable to close table");

    // Free the table.
    sky_table_free(table);
    
    // Free the database file.
    sky_database_free(database);
    
    return 0;

error:
    sky_table_free(table);
    sky_database_free(database);
    return -1;
}

