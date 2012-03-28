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

#ifndef _event_data_h
#define _event_data_h

#include <inttypes.h>

#include "bstring.h"


//==============================================================================
//
// Overview
//
//==============================================================================

// Event data is a simple hash of keys and values. Keys are stored as the id
// of the property they represent. Additional property information such as the
// property name is stored globally in the object file.


//==============================================================================
//
// Typedefs
//
//==============================================================================

typedef struct EventData {
    int16_t key;
    bstring value;
} EventData;


//==============================================================================
//
// Functions
//
//==============================================================================

//======================================
// Create/Destroy
//======================================

EventData *EventData_create(int16_t key, bstring value);

void EventData_destroy(EventData *event);


//======================================
// Serialization
//======================================

uint32_t EventData_get_serialized_length(EventData *data);

int EventData_serialize(EventData *data, FILE *file);

int EventData_deserialize(EventData *data, FILE *file);


#endif
