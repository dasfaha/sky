#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "bstring.h"
#include "dbg.h"
#include "server.h"
#include "version.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

#define SKY_DEFAULT_DATA_PATH "/usr/local/sky/data"

typedef struct Options {
    bstring path;
    int port;
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
        {"port", optional_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    // Parse command line options.
    while(1) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "p:", long_options, &option_index);
        
        // Check for end of options.
        if(c == -1) {
            break;
        }
        
        // Parse each option.
        switch(c) {
            case 'i': {
                options->port = atoi(optarg);
                break;
            }
        }
    }
    
    argc -= optind;
    argv += optind;

    // Default the path if one is not specified.
    if(argc < 1) {
        options->path = bfromcstr(SKY_DEFAULT_DATA_PATH); check_mem(options->path);
    }
    else {
        options->path = bfromcstr(argv[0]); check_mem(options->path);
    }

    // Default port.
    if(options->port < 0) {
        fprintf(stderr, "Error: Invalid port number.\n\n");
        exit(1);
    }

    return options;
    
error:
    exit(1);
}

void Options_free(Options *options)
{
    if(options) {
        bdestroy(options->path);
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
    printf("skyd " SKY_VERSION "\n");
    exit(0);
}

void usage()
{
    fprintf(stderr, "usage: skyd [OPTIONS] [PATH]\n\n");
    exit(0);
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

    // Create server.
    sky_server *server = sky_server_create(options->path);
    if(options->port > 0) {
        server->port = options->port;
    }
    
    // Clean up options.
    Options_free(options);
    
    // Display status.
    printf("Sky Server v%s\n", SKY_VERSION);
    printf("Listening on 0.0.0.0:%d, CTRL+C to stop\n", server->port);
    
    // Start server.
    sky_server_start(server);
    
    // Continuously accept connections.
    while(true) {
        sky_server_accept(server);
    }

    sky_server_stop(server);
    sky_server_free(server);

    return 0;
}

