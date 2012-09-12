#ifndef _sky_message_type_h
#define _sky_message_type_h

#include <inttypes.h>

#include "bstring.h"
#include "types.h"


//==============================================================================
//
// Definitions
//
//==============================================================================

//--------------------------------------
// Message Types
//--------------------------------------

#define SKY_MESSAGE_TYPE_EVENT     0x10000
#define SKY_MESSAGE_TYPE_PATH      0x20000
#define SKY_MESSAGE_TYPE_ACTION    0x30000
#define SKY_MESSAGE_TYPE_PROPERTY  0x40000

#define SKY_MESSAGE_TYPE_ADD       0x00001
#define SKY_MESSAGE_TYPE_UPD       0x00002
#define SKY_MESSAGE_TYPE_DEL       0x00003
#define SKY_MESSAGE_TYPE_GET       0x00004
#define SKY_MESSAGE_TYPE_ALL       0x00005
#define SKY_MESSAGE_TYPE_EACH      0x00006


//--------------------------------------
// Action Message Types
//--------------------------------------

#define SKY_MESSAGE_AADD (SKY_MESSAGE_TYPE_ACTION | SKY_MESSAGE_TYPE_ADD)
#define SKY_MESSAGE_AGET (SKY_MESSAGE_TYPE_ACTION | SKY_MESSAGE_TYPE_GET)
#define SKY_MESSAGE_AALL (SKY_MESSAGE_TYPE_ACTION | SKY_MESSAGE_TYPE_ALL)


//--------------------------------------
// Property Message Types
//--------------------------------------

#define SKY_MESSAGE_PADD (SKY_MESSAGE_TYPE_PROPERTY | SKY_MESSAGE_TYPE_ADD)
#define SKY_MESSAGE_PGET (SKY_MESSAGE_TYPE_PROPERTY | SKY_MESSAGE_TYPE_GET)
#define SKY_MESSAGE_PALL (SKY_MESSAGE_TYPE_PROPERTY | SKY_MESSAGE_TYPE_ALL)


//--------------------------------------
// Event Message Types
//--------------------------------------

#define SKY_MESSAGE_EADD (SKY_MESSAGE_TYPE_EVENT | SKY_MESSAGE_TYPE_ADD)


//--------------------------------------
// Path Message Types
//--------------------------------------

#define SKY_MESSAGE_PEACH (SKY_MESSAGE_TYPE_PATH | SKY_MESSAGE_TYPE_EACH)


#endif
