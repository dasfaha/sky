#include <stdlib.h>

#include "minipack.h"
#include "qip_cursor.h"
#include "sky_qip_module.h"
#include "dbg.h"


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a cursor.
sky_qip_cursor *sky_qip_cursor_create()
{
    sky_qip_cursor *cursor = malloc(sizeof(sky_qip_cursor));
    cursor->cursor = sky_cursor_create();
    return cursor;
}

// Frees a cursor.
//
// cursor - The cursor to free.
void sky_qip_cursor_free(sky_qip_cursor *cursor)
{
    if(cursor) {
        cursor->cursor = NULL;
        free(cursor);
    }
}

//--------------------------------------
// Cursor Management
//--------------------------------------

// Retrieves the next event in the cursor.
//
// module - The module.
// cursor - The cursor.
// event  - The event object to update.
//
// Returns nothing.
void sky_qip_cursor_next(qip_module *module, sky_qip_cursor *cursor,
                         sky_qip_event *event)
{
    int rc;
    size_t sz;
    check(module != NULL, "Module required");
    sky_qip_module *_module = (sky_qip_module*)module->context;
    check(_module != NULL, "Wrapped module required");

    // Update the action id on the event.
    sky_action_id_t action_id;
    rc = sky_cursor_get_action_id(cursor->cursor, &action_id);
    check(rc == 0, "Unable to retrieve action id");
    event->action_id = (int64_t)action_id;
    
    // Localize dynamic property info.
    int64_t property_count = _module->event_property_count;
    sky_property_id_t *property_ids = _module->event_property_ids;
    int64_t *property_offsets = _module->event_property_offsets;
    bstring *property_types = _module->event_property_types;

    // Read data block if we have properties attached to the wrapped module.
    if(property_count > 0) {
        // Retrieve pointer to the start of the data portion of the cursor.
        void *data_ptr = NULL;
        uint32_t data_length = 0;
        rc = sky_cursor_get_data_ptr(cursor->cursor, &data_ptr, &data_length);
        check(rc == 0, "Unable to retrieve cursor data pointer");

        // Loop over data section until we run out of data.
        void *ptr = data_ptr;
        while(ptr < data_ptr+data_length) {
            // Read property id.
            sky_property_id_t property_id = *((sky_property_id_t*)ptr);
            ptr += sizeof(property_id);
            
            // Initialize size to zero so we know if it was processed.
            sz = 0;

            // Loop over properties on event to check if we need to update.
            uint32_t i;
            for(i=0; i<property_count; i++) {
                if(property_id == property_ids[i]) {
                    void *property_value_ptr = ((void*)event) + property_offsets[i];
                    
                    // Parse the data by the data type set on the database property.
                    bstring property_type = property_types[i];
                    if(property_type == &SKY_DATA_TYPE_INT) {
                        *((int64_t*)property_value_ptr) = minipack_unpack_int(ptr, &sz);
                        check(sz != 0, "Unable to unpack event int data");
                        ptr += sz;
                        break;
                    }
                    else if(property_type == &SKY_DATA_TYPE_FLOAT) {
                        *((double*)property_value_ptr) = minipack_unpack_double(ptr, &sz);
                        check(sz != 0, "Unable to unpack event float data");
                        ptr += sz;
                        break;
                    }
                    else if(property_type == &SKY_DATA_TYPE_BOOLEAN) {
                        *((int64_t*)property_value_ptr) = minipack_unpack_bool(ptr, &sz);
                        check(sz != 0, "Unable to unpack event boolean data");
                        ptr += sz;
                        break;
                    }
                    else if(property_type == &SKY_DATA_TYPE_STRING) {
                        qip_string *string_value = (qip_string*)property_value_ptr;
                        string_value->length = minipack_unpack_raw(ptr, &sz);
                        check(sz != 0, "Unable to unpack event string data");
                        ptr += sz;
                        string_value->data = ptr;
                        ptr += string_value->length;
                        break;
                    }
                }
            }
            
            // If the property was not processed then jump ahead to the next
            // property value in the event.
            if(sz == 0) {
                sz = minipack_sizeof_elem_and_data(ptr);
                check(sz > 0, "Invalid data found in event");
                ptr += sz;
            }
        }
    }
    
    // Move to the next event in the cursor.
    sky_cursor_next(cursor->cursor);
    
    return;
    
error:
    cursor->cursor->eof = true;
    return;
}

// Checks whether the cursor is at the end.
//
// module - The module.
// cursor - The cursor.
//
// Returns a flag stating if the cursor is done.
bool sky_qip_cursor_eof(qip_module *module, sky_qip_cursor *cursor)
{
    check(module != NULL, "Module required");
    return cursor->cursor->eof;

error:
    return true;
}
