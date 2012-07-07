#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>

#include "bstring.h"
#include "dbg.h"
#include "database.h"
#include "table.h"
#include "version.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The sky-gen application is used for generating random datasets for
// performance testing. It allows a user to set options such as the number of
// paths to generate and the average number of events to generate per path.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Options {
    bstring path;
    bstring object_type;
    int32_t path_count;
    int32_t avg_event_count;
    int32_t action_count;
    int32_t seed;
} Options;


//==============================================================================
//
// Command Line Arguments
//
//==============================================================================

Options *parseopts(int argc, char **argv)
{
    Options *options = (Options*)calloc(1, sizeof(Options));
    check_mem(options);
    
    // Command line options.
    struct option long_options[] = {
        {"object-type", required_argument, 0, 'o'},
        {"path-count", required_argument, 0, 'p'},
        {"avg-event-count", required_argument, 0, 'e'},
        {"action-count", required_argument, 0, 'a'},
        {"seed", optional_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    // Parse command line options.
    while(1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "o:p:e:s:a:", long_options, &option_index);
        
        // Check for end of options.
        if(c == -1) {
            break;
        }
        
        // Parse each option.
        switch(c) {
            case 'o': {
                options->object_type = bfromcstr(optarg);
                check_mem(options->object_type);
                break;
            }
            
            case 'p': {
                options->path_count = atoi(optarg);
                break;
            }

            case 'e': {
                options->avg_event_count = atoi(optarg);
                break;
            }

            case 'a': {
                options->action_count = atoi(optarg);
                break;
            }

            case 's': {
                options->seed = atoi(optarg);
                break;
            }
        }
    }
    
    argc -= optind;
    argv += optind;

    // Retrieve path as first non-getopts option.
    if(argc < 1) {
        fprintf(stderr, "Error: Database path required.\n\n");
        exit(1);
    }
    options->path = bfromcstr(argv[0]);

    // Validate input.
    if(options->object_type == NULL) {
        fprintf(stderr, "Error: Object type (-o) is required.\n\n");
        exit(1);
    }

    // Default input.
    if(options->path_count <= 0) {
        options->path_count = 100;
    }
    if(options->avg_event_count <= 0) {
        options->avg_event_count = 10;
    }
    if(options->action_count <= 0) {
        options->action_count = 100;
    }
    
    // Randomize seed if not provided.
    if(options->seed == 0) {
        options->seed = time(NULL);
        fprintf(stderr, "Generating random seed: %d\n", options->seed);
    }

    return options;
    
error:
    exit(1);
}

void Options_free(Options *options)
{
    if(options) {
        bdestroy(options->object_type);
        options->object_type = NULL;
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
    printf("sky-gen " SKY_VERSION "\n");
    exit(0);
}

void usage()
{
    fprintf(stderr, "usage: sky-gen [OPTIONS] [PATH]\n\n");
    exit(0);
}

//==============================================================================
//
// Event Management
//
//==============================================================================

// Generates a database with random data at a given path.
//
// options - A list of options to use while generating the database.
void generate(Options *options)
{
    int rc;
    sky_event *event = NULL;
    
    // Seed the randomizer.
    srandom(options->seed);
    
    // Create database.
    sky_database *database = sky_database_create(options->path);
    check_mem(database);
    
    // Open table.
    sky_table *table = sky_table_create(database, options->object_type);
    check_mem(table);
    
    check(sky_table_open(table) == 0, "Unable to open table");
    check(sky_table_lock(table) == 0, "Unable to lock table");

    // Loop over paths and create events.
    int i, j;
    for(i=0; i<options->path_count; i++) {
        sky_object_id_t object_id = (sky_timestamp_t)(i+1);
        int32_t event_count = (random() % ((options->avg_event_count*2) - 1)) + 1;
        
        // Create a bunch of events.
        for(j=0; j<event_count; j++) {
            sky_timestamp_t timestamp = random() % INT64_MAX;
            sky_action_id_t action_id = (random() % options->action_count) + 1;
            event = sky_event_create(timestamp, object_id, action_id);
            rc = sky_table_add_event(table, event);
            check(rc == 0, "Unable to add event: ts:%lld, oid:%lld, action:%lld", event->timestamp, event->object_id, event->action_id);
            sky_event_free(event);
        }
    }
    
    // Close table
    check(sky_table_unlock(table) == 0, "Unable to unlock table");
    check(sky_table_close(table) == 0, "Unable to close table");
    
    // Clean up
    sky_database_free(database);
    sky_table_free(table);
    
    return;
    
error:
    sky_event_free(event);
    sky_table_close(table);

    sky_database_free(database);
    sky_table_free(table);
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

    // Start time.
    time_t t0 = time(NULL);

    // Generate database.
    generate(options);

    // Show wall clock time.
    printf("Elapsed Time: %ld seconds\n", (time(NULL)-t0));

    // Clean up.
    Options_free(options);
    
    return 0;
}

