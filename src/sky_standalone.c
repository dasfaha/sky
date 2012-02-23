#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

// =============================================================================
//
// Command Line Arguments
//
// =============================================================================

void parseopts(int argc, char **argv)
{
    int c;
    int version_flag;
    
    // Command line options.
    struct option long_options[] = {
        {"version", no_argument, &version_flag, 1},
        {0, 0, 0, 0}
    };

    // Parse command line options.
    while(1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        
        if(c == -1) {
            break;
        }
    }
    
    // Print version number if requested.
    if(version_flag) {
        printf("sky-standalone 0.1.0\n");
    }
}

void usage()
{
    fprintf(stderr, "usage: sky-standalone [--version]\n\n");
    exit(0);
}


// =============================================================================
//
// Main
//
// =============================================================================

int main(int argc, char **argv)
{
    // Parse command line options.
    parseopts(argc, argv);

    return 0;
}

