#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include "bstring.h"
#include "dbg.h"
#include "database.h"
#include "table.h"
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
    bstring table_name;
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
        {"table-name", required_argument, 0, 't'},
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
            case 't': {
                options->table_name = bfromcstr(optarg);
                check_mem(options->table_name);
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
    /*
    if(options->object_type == NULL) {
        fprintf(stderr, "Error: Object type (-o) is required.\n\n");
        exit(1);
    }
    */

    // Default input.
    if(options->iterations <= 0) {
        options->iterations = 1;
    }

    return options;
    
error:
    exit(1);
}

void Options_free(Options *options)
{
    if(options) {
        bdestroy(options->path);
        bdestroy(options->table_name);
        options->table_name = NULL;
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
    sky_event *event = NULL;
    uint32_t event_count = 0;
    int32_t action_count = 100;     // TODO: Retrieve action count from actions file.
    
    // Initialize table.
    sky_table *table = sky_table_create(); check_mem(table);
    rc = sky_table_set_path(table, options->path);
    check(rc == 0, "Unable to set path on table");
    
    // Open table
    rc = sky_table_open(table);
    check(rc == 0, "Unable to open table");

    // Loop for desired number of iterations.
    int i;
    for(i=0; i<options->iterations; i++) {
        sky_path_iterator iterator;
        sky_path_iterator_init(&iterator);
        
        // Attach data file.
        rc = sky_path_iterator_set_data_file(&iterator, table->data_file);
        check(rc == 0, "Unable to initialze path iterator");
        
        // Create a square matrix of structs.
        Step *steps = calloc(action_count*action_count, sizeof(Step));
    
        // Iterate over each path.
        while(!iterator.eof) {
            sky_action_id_t action_id, prev_action_id;

            // Retrieve the path pointer.
            void *path_ptr;
            rc = sky_path_iterator_get_ptr(&iterator, &path_ptr);
            check(rc == 0, "Unable to retrieve the path iterator pointer");
        
            // Initialize the cursor.
            sky_cursor cursor;
            sky_cursor_init(&cursor);
            rc = sky_cursor_set_path(&cursor, path_ptr);
            check(rc == 0, "Unable to set cursor path");

            // Increment total event count.
            event_count++;
            
            // Initialize the previous action.
            rc = sky_cursor_get_action_id(&cursor, &prev_action_id);
            check(rc == 0, "Unable to retrieve first action");

            // Find next event.
            rc = sky_cursor_next(&cursor);
            check(rc == 0, "Unable to find next event");

            // Loop over each event in the path.
            while(!cursor.eof) {
                // Increment total event count.
                event_count++;

                // Retrieve action.
                rc = sky_cursor_get_action_id(&cursor, &action_id);
                check(rc == 0, "Unable to retrieve first action");

                // Aggregate step information.
                int32_t index = ((prev_action_id-1)*action_count) + (action_id-1);
                steps[index].count++;

                // Assign current action as previous action.
                prev_action_id = action_id;

                // Find next event.
                rc = sky_cursor_next(&cursor);
                check(rc == 0, "Unable to find next event");
            }
            
            rc = sky_path_iterator_next(&iterator);
            check(rc == 0, "Unable to find next path");
        }
        
        // Show DAG data.
        //int x;
        //int total=0;
        //for(x=0; x<action_count*action_count; x++) {
        //    printf("%06d %d\n", x, steps[x].count);
        //    total += steps[x].count;
        //}
        //printf("total: %d\n", total);
    }
    
    // Clean up
    rc = sky_table_close(table);
    check(rc == 0, "Unable to close table");
    sky_table_free(table);

    // Show stats.
    printf("Total events processed: %d\n", event_count);
    
    return;
    
error:
    sky_event_free(event);
    sky_table_free(table);
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
    Options_free(options);
    
    return 0;
}

