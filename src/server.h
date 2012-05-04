#ifndef _server_h
#define _server_h

#include <inttypes.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "bstring.h"
#include "database.h"
#include "object_file.h"
#include "event.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// The server acts as the interface to external applications. It communicates
// over TCP sockets using a specific Sky protocol. See the message.h file for
// more detail on the protocol.


//==============================================================================
//
// Definitions
//
//==============================================================================

#define SKY_DEFAULT_PORT 8585

#define SKY_LISTEN_BACKLOG 511


//==============================================================================
//
// Typedefs
//
//==============================================================================

// The various states that the server can be in.
typedef enum sky_server_state_e {
    SKY_SERVER_STATE_STOPPED,
    SKY_SERVER_STATE_RUNNING,
} sky_server_state_e;

typedef struct sky_server {
    sky_server_state_e state;
    bstring path;
    int port;
    struct sockaddr_in* sockaddr;
    int socket;
} sky_server;




//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Lifecycle
//======================================

sky_server *sky_server_create(bstring path);

void sky_server_free(sky_server *server);


//======================================
// State
//======================================

int sky_server_start(sky_server *server);

int sky_server_stop(sky_server *server);


//======================================
// Message Processing
//======================================

int sky_server_process_eadd_message(sky_server *server, int socket,
                                    void *buffer);

#endif
