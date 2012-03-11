/*
 * Copyright (c) 2011 Ben Johnson, http://skylandlabs.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "bstring.h"
#include "dbg.h"
#include "database.h"
#include "object_file.h"
#include "timestamp.h"

//==============================================================================
//
// Overview
//
//==============================================================================

/*
 * The Sky Standalone client is primarily used for testing and for users who
 * wish to play with the database without actually running a server. It is not
 * meant to be run in a production environment. That being said, the standalone
 * CLI provides nearly all the functionality of the regular Sky server.
 *
 * For more information on running the CLI, please refer to the README.
 */

//==============================================================================
//
// Typedefs
//
//==============================================================================

enum e_command {
    CMD_SHOW_VERSION = 1,
    CMD_ADD_EVENT = 2
};

typedef struct Options {
    enum e_command command;
    bstring database;
    bstring object_type;
    int64_t object_id;
    bstring timestamp;
    bstring action;
    bstring *data;
    int data_count;
} Options;


//==============================================================================
//
// Command Line Arguments
//
//==============================================================================

Options *parseopts(int argc, char **argv)
{
    int c;
    int command;
    Options *options = (Options*)calloc(1, sizeof(Options));
    check_mem(options);
    
    // Command line options.
    struct option long_options[] = {
        {"version", no_argument, &command, CMD_SHOW_VERSION},
        {"add-event", no_argument, &command, CMD_ADD_EVENT},
        {"database", required_argument, 0, 'd'},
        {"object-type", required_argument, 0, 't'},
        {"object-id", required_argument, 0, 'i'},
        {"timestamp", required_argument, 0, 'T'},
        {"action", required_argument, 0, 'a'},
        {"data", required_argument, 0, 'D'},
        {0, 0, 0, 0}
    };

    // Parse command line options.
    while(1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "d:t:i:T:a:D:", long_options, &option_index);
        
        // Check for end of options.
        if(c == -1) {
            break;
        }
        
        // Parse each option.
        switch(c) {
        case 'd':
            options->database = bfromcstr(optarg); check_mem(options->database);
            break;
            
        case 't':
            options->object_type = bfromcstr(optarg); check_mem(options->object_type);
            break;
            
        case 'i':
            options->object_id = atoll(optarg);
            break;

        case 'T':
            options->timestamp = bfromcstr(optarg); check_mem(options->timestamp);
            break;

        case 'a':
            options->action = bfromcstr(optarg); check_mem(options->action);
            break;

        case 'D':
            // Increment array allocation.
            options->data_count++;
            reallocf(options->data, sizeof(bstring)*options->data_count);
            check_mem(options->data);
            
            // Append data to array.
            bstring data = bfromcstr(optarg); check_mem(data);
            options->data[options->data_count-1] = data;
            break;
        }
    }
    
    argc -= optind;
    argv += optind;

    // Set command on options.
    options->command = command;
    
    return options;
    
error:
    exit(1);
}

/**
 * Releases memory held by an Options struct.
 */
void Options_destroy(Options *options)
{
    int i;

    if(options) {
        bdestroy(options->database); options->database = NULL;
        bdestroy(options->object_type); options->object_type = NULL;
        bdestroy(options->timestamp); options->timestamp = NULL;
        bdestroy(options->action); options->action = NULL;

        options->object_id = 0;

        // Clean up data elements.
        for(i=0; i<options->data_count; i++) {
            bdestroy((bstring)&options[i]);
        }
        free(options->data); options->data = NULL;

        free(options);
    }
}


//==============================================================================
//
// Usage & Version
//
//==============================================================================

void print_version()
{
    printf("sky-standalone 0.1.0\n");
    exit(0);
}

void usage()
{
    fprintf(stderr, "usage: sky-standalone [--version] [--add-event]\n\n");
    exit(0);
}

//==============================================================================
//
// Event Management
//
//==============================================================================

/**
 * Adds an event to an object file in a database.
 */
void add_event(Options *options)
{
    int rc;
    int64_t ts;
    bstring path = bstrcpy(options->database);
    bstring timestamp = bstrcpy(options->timestamp);
    Event *event = NULL;
    
    // Validate options.
    if(!options->object_type) {
        fprintf(stderr, "Object type is required.\n"); exit(1);
    }
    if(!options->object_id) {
        fprintf(stderr, "Object id is required.\n"); exit(1);
    }
    if(!options->action) {
        fprintf(stderr, "Action is required.\n"); exit(1);
    }

    // Default database to current working directory.
    if(path == NULL) {
        char *cwd = getcwd(NULL, 0);
        check(cwd != NULL, "Current working directory could not be found");
        path = bfromcstr(cwd);
        free(cwd);
    }
    
    // Create database reference.
    Database *database = Database_create(path);
    bdestroy(path);
    check_mem(database);
    
    // Open object file reference.
    ObjectFile *object_file = ObjectFile_create(database, options->object_type);
    check_mem(object_file);
    
    // Use current time if timestamp was not passed in.
    if(timestamp == NULL) {
        rc = Timestamp_now(&ts);
        check(rc == 0, "Can not determine current timestamp");
    }
    // Parse ISO8601 timestamp.
    else {
        rc = Timestamp_parse(timestamp, &ts);
        check(rc == 0, "Could not parse timestamp")
    }
    
    // Create an event object.
    event = Event_create(ts, options->object_id, options->action);

    // TODO: Set data.

    // Print parameters.
    printf("DATABASE:    %s\n", bdata(database->path));
    printf("OBJECT TYPE: %s\n", bdata(object_file->name));
    printf("OBJECT ID:   %lld\n", event->object_id);
    printf("ACTION:      %s\n", bdata(event->action));
    printf("TIMESTAMP:   %lld\n", event->timestamp);
    printf("\n");
    
    // Add the event to the object file.
    ObjectFile_open(object_file);
    ObjectFile_add_event(object_file, event);
    ObjectFile_close(object_file);
    
    // Clean up
    Database_destroy(database);
    ObjectFile_destroy(object_file);
    Event_destroy(event);
    
    return;
    
error:
    bdestroy(path);
    bdestroy(timestamp);
    ObjectFile_close(object_file);

    Database_destroy(database);
    ObjectFile_destroy(object_file);
    Event_destroy(event);
}

//==============================================================================
//
// Main
//
//==============================================================================

int main(int argc, char **argv)
{
    // Parse command line options.
    Options *options = parseopts(argc, argv);

    // Determine what command to execute.
    switch(options->command) {
    case CMD_SHOW_VERSION:
        print_version();
        break;
        
    case CMD_ADD_EVENT:
        add_event(options);
        break;
        
    default:
        fprintf(stderr, "Missing command: version, add-event");
        break;
    }

    // Clean up.
    Options_destroy(options);
    
    return 0;
}

