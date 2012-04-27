#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include "bstring.h"
#include "dbg.h"
#include "database.h"
#include "object_file.h"
#include "cursor.h"
#include "path_iterator.h"
#include "version.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The sky-bench application is used for benchmarking databases in different
// ways. The tool currently only supports basic iteration through the entire
// database.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct Options {
    bstring path;
    bstring object_type;
    int32_t iterations;
} Options;


// A data structure used for aggregation information between events in the path.
typedef struct Step {
    int32_t count;
} Step;

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
        {"iterations", required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };

    // Parse command line options.
    while(1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "o:i:", long_options, &option_index);
        
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
            
            case 'i': {
                options->iterations = atoi(optarg);
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
    if(options->iterations <= 0) {
        options->iterations = 1;
    }

    return options;
    
error:
    exit(1);
}

void Options_destroy(Options *options)
{
    if(options) {
        bdestroy(options->path);
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
    printf("sky-bench " SKY_VERSION "\n");
    exit(0);
}

void usage()
{
    fprintf(stderr, "usage: sky-bench [OPTIONS] [PATH]\n\n");
    exit(0);
}


//==============================================================================
//
// Benchmark
//
//==============================================================================

// Executes the benchmark over the database to compute step counts in order to
// generate a directed acyclic graph (DAG).
//
// options - A list of options to use.
void benchmark_dag(Options *options)
{
    int rc;
    Event *event = NULL;
    uint32_t event_count = 0;
    int32_t action_count = 100;     // TODO: Retrieve action count from actions file.
    
    // Create database.
    Database *database = Database_create(options->path);
    check_mem(database);
    
    // Open object file.
    ObjectFile *object_file = ObjectFile_create(database, options->object_type);
    check_mem(object_file);
    
    check(ObjectFile_open(object_file) == 0, "Unable to open object file");
    check(ObjectFile_lock(object_file) == 0, "Unable to lock object file");

    // Loop for desired number of iterations.
    int i;
    for(i=0; i<options->iterations; i++) {
        // Create a path iterator for the object file.
        Cursor *cursor = Cursor_create();
        PathIterator *iterator = PathIterator_create(object_file);
        PathIterator_next(iterator, cursor);
        
        // Create a square matrix of structs.
        Step *steps = calloc(action_count*action_count, sizeof(Step));
    
        // Iterate over each path.
        while(!iterator->eof) {
            int32_t action_id, prev_action_id;

            // Increment total event count.
            event_count++;
            
            // Initialize the previous action.
            rc = Cursor_get_action(cursor, &prev_action_id);
            check(rc == 0, "Unable to retrieve first action");

            // Find first event.
            rc = Cursor_next_event(cursor);
            check(rc == 0, "Unable to find next event");

            // Loop over each event in the path.
            while(!cursor->eof) {
                // Increment total event count.
                event_count++;

                // Retrieve action.
                rc = Cursor_get_action(cursor, &action_id);
                check(rc == 0, "Unable to retrieve first action");

                // Aggregate step information.
                int32_t index = ((prev_action_id-1)*action_count) + (action_id-1);
                steps[index].count++;

                // Assign current action as previous action.
                prev_action_id = action_id;

                // Find next event.
                rc = Cursor_next_event(cursor);
                check(rc == 0, "Unable to find next event");
            }
            
            rc = PathIterator_next(iterator, cursor);
            check(rc == 0, "Unable to find next path");
        }
        
        // Clean up.
        Cursor_destroy(cursor);
        PathIterator_destroy(iterator);

        // Show DAG data.
        //int x;
        //int total=0;
        //for(x=0; x<action_count*action_count; x++) {
        //    printf("%06d %d\n", x, steps[x].count);
        //    total += steps[x].count;
        //}
        //printf("total: %d\n", total);
    }
    
    // Close object file
    check(ObjectFile_unlock(object_file) == 0, "Unable to unlock object file");
    check(ObjectFile_close(object_file) == 0, "Unable to close object file");
    
    // Clean up
    Database_destroy(database);
    ObjectFile_destroy(object_file);

    // Show stats.
    printf("Total events processed: %d\n", event_count);
    
    return;
    
error:
    Event_destroy(event);
    ObjectFile_close(object_file);

    Database_destroy(database);
    ObjectFile_destroy(object_file);
}


//==============================================================================
//
// Main
//
//==============================================================================

int main(int argc, char **argv)
{
    struct timeval tv;

    // Parse command line options.
    Options *options = parseopts(argc, argv);

    // Start time.
    gettimeofday(&tv, NULL);
    int64_t t0 = (tv.tv_sec*1000) + (tv.tv_usec/1000);

    // Benchmark computation of a DAG.
    benchmark_dag(options);

    // End time.
    gettimeofday(&tv, NULL);
    int64_t t1 = (tv.tv_sec*1000) + (tv.tv_usec/1000);

    // Show wall clock time.
    printf("Elapsed Time: %.3f seconds\n", ((float)(t1-t0))/1000);

    // Clean up.
    Options_destroy(options);
    
    return 0;
}

