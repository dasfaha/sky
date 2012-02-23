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

