#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>

#include "dbg.h"
#include "mem.h"
#include "endian.h"
#include "bstring.h"
#include "file.h"
#include "database.h"
#include "block.h"
#include "table.h"

//==============================================================================
//
// Forward Declarations
//
//==============================================================================

//--------------------------------------
// Locking
//--------------------------------------

int sky_table_lock(sky_table *table);

int sky_table_unlock(sky_table *table);


//--------------------------------------
// Data file
//--------------------------------------

int sky_table_load_data_file(sky_table *table);

int sky_table_unload_data_file(sky_table *table);


//--------------------------------------
// Action file
//--------------------------------------

int sky_table_load_action_file(sky_table *table);

int sky_table_unload_action_file(sky_table *table);

//--------------------------------------
// Property file
//--------------------------------------

int sky_table_load_property_file(sky_table *table);

int sky_table_unload_property_file(sky_table *table);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------


// Creates a reference to an table.
// 
// Returns a reference to the new table if successful. Otherwise returns
// null.
sky_table *sky_table_create()
{
    sky_table *table = calloc(sizeof(sky_table), 1); check_mem(table);
    return table;
    
error:
    sky_table_free(table);
    return NULL;
}

// Removes an table reference from memory.
//
// table - The table to free.
void sky_table_free(sky_table *table)
{
    if(table) {
        bdestroy(table->name);
        table->name = NULL;
        bdestroy(table->path);
        table->path = NULL;
        sky_table_unload_action_file(table);
        sky_table_unload_property_file(table);
        free(table);
    }
}


//--------------------------------------
// Path Management
//--------------------------------------

// Sets the file path of a table.
//
// table - The table.
// path  - The file path to set.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_set_path(sky_table *table, bstring path)
{
    check(table != NULL, "Table required");

    if(table->path) {
        bdestroy(table->path);
    }
    
    table->path = bstrcpy(path);
    if(path) check_mem(table->path);

    return 0;

error:
    table->path = NULL;
    return -1;
}


//--------------------------------------
// Data file management
//--------------------------------------

// Initializes and opens the data file on the table.
//
// table - The table to initialize the data file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_load_data_file(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path required");
    
    // Unload any existing data file.
    sky_table_unload_data_file(table);
    
    // Initialize table space (0).
    bstring tablespace_path = bformat("%s/0", bdata(table->path));
    if(!sky_file_exists(tablespace_path)) {
        rc = mkdir(bdata(tablespace_path), S_IRWXU);
        check(rc == 0, "Unable to create tablespace directory: %s", bdata(tablespace_path));
    }
    bdestroy(tablespace_path);
    
    // Initialize data file.
    table->data_file = sky_data_file_create();
    check_mem(table->data_file);
    table->data_file->path = bformat("%s/0/data", bdata(table->path));
    check_mem(table->data_file->path);
    table->data_file->header_path = bformat("%s/0/header", bdata(table->path));
    check_mem(table->data_file->header_path);
    
    // Initialize settings on the block.
    if(table->default_block_size > 0) {
        table->data_file->block_size = table->default_block_size;
    }
    
    // Load data
    rc = sky_data_file_load(table->data_file);
    check(rc == 0, "Unable to load data file");

    return 0;
error:
    bdestroy(tablespace_path);
    sky_table_unload_data_file(table);
    return -1;
}

// Initializes and opens the data file on the table.
//
// table - The table to initialize the data file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_unload_data_file(sky_table *table)
{
    check(table != NULL, "Table required");

    if(table->data_file) {
        sky_data_file_free(table->data_file);
        table->data_file = NULL;
    }

    return 0;
error:
    return -1;
}


//--------------------------------------
// Action file management
//--------------------------------------

// Initializes and opens the action file on the table.
//
// table - The table to initialize the action file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_load_action_file(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path required");
    
    // Unload any existing action file.
    sky_table_unload_action_file(table);
    
    // Initialize action file.
    table->action_file = sky_action_file_create();
    check_mem(table->action_file);
    table->action_file->path = bformat("%s/actions", bdata(table->path));
    check_mem(table->action_file->path);
    
    // Load data
    rc = sky_action_file_load(table->action_file);
    check(rc == 0, "Unable to load actions");

    return 0;
error:
    sky_table_unload_action_file(table);
    return -1;
}

// Initializes and opens the action file on the table.
//
// table - The table to initialize the action file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_unload_action_file(sky_table *table)
{
    check(table != NULL, "Table required");

    if(table->action_file) {
        sky_action_file_free(table->action_file);
        table->action_file = NULL;
    }

    return 0;
error:
    return -1;
}


//--------------------------------------
// Property file management
//--------------------------------------

// Initializes and opens the property file on the table.
//
// table - The table to initialize the property file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_load_property_file(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path required");
    
    // Unload any existing property file.
    sky_table_unload_property_file(table);
    
    // Initialize property file.
    table->property_file = sky_property_file_create();
    check_mem(table->property_file);
    table->property_file->path = bformat("%s/properties", bdata(table->path));
    check_mem(table->property_file->path);
    
    // Load data
    rc = sky_property_file_load(table->property_file);
    check(rc == 0, "Unable to load properties");

    return 0;
error:
    sky_table_unload_property_file(table);
    return -1;
}

// Initializes and opens the property file on the table.
//
// table - The table to initialize the property file for.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_unload_property_file(sky_table *table)
{
    check(table != NULL, "Table required");

    if(table->property_file) {
        sky_property_file_free(table->property_file);
        table->property_file = NULL;
    }

    return 0;
error:
    return -1;
}


//--------------------------------------
// State
//--------------------------------------

// Opens the table for reading and writing events.
// 
// table - The table to open.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_open(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required");
    check(table->path != NULL, "Table path is required");
    check(!table->opened, "Table is already open");

    // Create directory if it doesn't exist.
    if(!sky_file_exists(table->path)) {
        rc = mkdir(bdata(table->path), S_IRWXU);
        check(rc == 0, "Unable to create table directory: %s", bdata(table->path));
    }

    // Obtain a lock.
    rc = sky_table_lock(table);
    check(rc == 0, "Unable to obtain lock");

    // Load data file.
    rc = sky_table_load_data_file(table);
    check(rc == 0, "Unable to load data file");
    
    // Load action file.
    rc = sky_table_load_action_file(table);
    check(rc == 0, "Unable to load action file");
    
    // Load property file.
    rc = sky_table_load_property_file(table);
    check(rc == 0, "Unable to load property file");
    
    // Flag the table as open.
    table->opened = true;

    return 0;

error:
    sky_table_close(table);
    return -1;
}

// Closes an table.
//
// table - The table to close.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_close(sky_table *table)
{
    int rc;
    check(table != NULL, "Table required to close");

    // Unload data file.
    rc = sky_table_unload_data_file(table);
    check(rc == 0, "Unable to unload data file");

    // Unload action data.
    rc = sky_table_unload_action_file(table);
    check(rc == 0, "Unable to unload action file");

    // Unload property data.
    rc = sky_table_unload_property_file(table);
    check(rc == 0, "Unable to unload property file");

    // Update state to closed.
    table->opened = false;

    // Release the lock.
    rc = sky_table_unlock(table);
    check(rc == 0, "Unable to remove lock");

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Locking
//--------------------------------------

// Creates a lock file for the table.
// 
// table - The table to lock.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_lock(sky_table *table)
{
    FILE *file;
    check(table != NULL, "Table required to lock");

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_LOCK_NAME); check_mem(path);

    // Raise error if table is already locked.
    check(!sky_file_exists(path), "Cannot obtain lock: %s", bdata(path));

    // Write pid to lock file.
    file = fopen(bdata(path), "w");
    check(file, "Failed to open lock file: %s",  bdata(path));
    check(fprintf(file, "%d", getpid()) > 0, "Error writing lock file: %s",  bdata(path));
    fclose(file);

    // Clean up.
    bdestroy(path);

    return 0;

error:
    if(file) fclose(file);
    bdestroy(path);
    return -1;
}

// Unlocks an table.
// 
// table - The table to unlock.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_unlock(sky_table *table)
{
    FILE *file;

    // Validate arguments.
    check(table != NULL, "Table required to unlock");

    // Construct path to lock.
    bstring path = bformat("%s/%s", bdata(table->path), SKY_LOCK_NAME); check_mem(path);

    // If file exists, check its PID and then attempt to remove it.
    if(sky_file_exists(path)) {
        // Read PID from lock.
        pid_t pid = 0;
        file = fopen(bdata(path), "r");
        check(file, "Failed to open lock file: %s",  bdata(path));
        check(fscanf(file, "%d", &pid) > 0, "Error reading lock file: %s", bdata(path));
        fclose(file);

        // Make sure we are removing a lock we created.
        check(pid == getpid(), "Cannot remove lock from another process (PID #%d): %s", pid, bdata(path));

        // Remove lock.
        check(unlink(bdata(path)) == 0, "Unable to remove lock: %s", bdata(path));
    }

    // Clean up.
    bdestroy(path);

    return 0;

error:
    if(file) fclose(file);
    bdestroy(path);
    return -1;
}


//--------------------------------------
// Event Management
//--------------------------------------

// Adds an event to the table.
//
// table - The table to add the event to..
// event - The event to add.
//
// Returns 0 if successful, otherwise returns -1.
int sky_table_add_event(sky_table *table, sky_event *event)
{
    int rc;
    check(table != NULL, "Table required");
    check(event != NULL, "Event required");
    check(table->opened, "Table must be open to add an event");

    // Delegate to the data file.
    rc = sky_data_file_add_event(table->data_file, event);
    check(rc == 0, "Unable to add event to data file");
    
    return 0;

error:
    return -1;
}

