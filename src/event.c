/*
 * Copyright (c) 2012 Ben Johnson, http://skylandlabs.com
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

#include <stdlib.h>

#include "dbg.h"
#include "bstring.h"


//==============================================================================
//
// Event Management
//
//==============================================================================

/*
 * Creates a reference to an event.
 *
 * timestamp - When the event occurred (in milliseconds since midnight Jan 1,
 *             1970 UTC).
 * object_id - The identifier for the object that the event is related to.
 * action    - The name of the action that was performed.
 */
Event *Event_create(long long timestamp, long long object_id, bstring action)
{
    Event *event;
    
    event = malloc(sizeof(Event));
    event->timestamp = timestamp;
    event->object_id = object_id;
    event->action    = bstrcpy(action); check_mem(event->action);
    event->data = NULL;

    return event;
    
error:
    Event_destroy(event);
    return NULL;
}

/*
 * Removes an event reference from memory.
 */
void Event_destroy(Event *event)
{
    if(event) {
        bdestroy(event->action);
        free(event);
    }
}
