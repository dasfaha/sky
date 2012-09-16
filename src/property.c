#include <stdlib.h>
#include <inttypes.h>

#include "dbg.h"
#include "bstring.h"
#include "property.h"
#include "minipack.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_property_update_type(sky_property *property);


//==============================================================================
//
// Definitions
//
//==============================================================================

struct tagbstring SKY_DATA_TYPE_INT = bsStatic("Int");

struct tagbstring SKY_DATA_TYPE_FLOAT = bsStatic("Float");

struct tagbstring SKY_DATA_TYPE_BOOLEAN = bsStatic("Boolean");

struct tagbstring SKY_DATA_TYPE_STRING = bsStatic("String");


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates a reference to an property.
// 
// Returns a reference to the new property if successful.
sky_property *sky_property_create()
{
    sky_property *property = calloc(sizeof(sky_property), 1);
    check_mem(property);
    return property;
    
error:
    sky_property_free(property);
    return NULL;
}

// Removes an property reference from memory.
//
// property - The property to free.
void sky_property_free(sky_property *property)
{
    if(property) {
        property->property_file = NULL;
        property->id = 0;
        if(property->data_type) bdestroy(property->data_type);
        property->data_type = NULL;
        if(property->name) bdestroy(property->name);
        property->name = NULL;
        free(property);
    }
}


//--------------------------------------
// Serialization
//--------------------------------------

// Calculates the total number of bytes needed to store the property.
//
// property - The property.
//
// Returns the number of bytes required to store the property.
size_t sky_property_sizeof(sky_property *property)
{
    size_t sz = 0;
    sz += minipack_sizeof_map(2);
    sz += minipack_sizeof_raw(strlen("id")) + strlen("id");
    sz += minipack_sizeof_int(property->id);
    sz += minipack_sizeof_raw(strlen("type")) + strlen("type");
    sz += minipack_sizeof_uint(property->type);
    sz += minipack_sizeof_raw(strlen("dataType")) + strlen("dataType");
    sz += blength(property->data_type);
    sz += minipack_sizeof_raw(strlen("name")) + strlen("name");
    sz += blength(property->name);
    return sz;
}

// Serializes a property to a file stream.
//
// property - The property.
// file   - The file stream to write to.
//
// Returns 0 if successful, otherwise returns -1.
int sky_property_pack(sky_property *property, FILE *file)
{
    int rc;
    size_t sz;
    check(property != NULL, "Property required");
    check(file != NULL, "File stream required");

    struct tagbstring id_str = bsStatic("id");
    struct tagbstring type_str = bsStatic("type");
    struct tagbstring data_type_str = bsStatic("dataType");
    struct tagbstring name_str = bsStatic("name");

    // Update the type just in case.
    rc = sky_property_update_type(property);
    check(rc == 0, "Unable to update property type");

    // Map
    minipack_fwrite_map(file, 4, &sz);
    check(sz > 0, "Unable to write map");
    
    // ID
    check(sky_minipack_fwrite_bstring(file, &id_str) == 0, "Unable to write id key");
    minipack_fwrite_int(file, property->id, &sz);
    check(sz > 0, "Unable to write id value");

    // Type
    check(sky_minipack_fwrite_bstring(file, &type_str) == 0, "Unable to write type key");
    minipack_fwrite_uint(file, property->type, &sz);
    check(sz > 0, "Unable to write type value");

    // Data Type
    check(sky_minipack_fwrite_bstring(file, &data_type_str) == 0, "Unable to write data type key");
    check(sky_minipack_fwrite_bstring(file, property->data_type) == 0, "Unable to write data type value");

    // Name
    check(sky_minipack_fwrite_bstring(file, &name_str) == 0, "Unable to write name key");
    check(sky_minipack_fwrite_bstring(file, property->name) == 0, "Unable to write name value");

    return 0;

error:
    return -1;
}

// Deserializes an property from a file stream.
//
// property - The property.
// file   - The file stream to read from.
//
// Returns 0 if successful, otherwise returns -1.
int sky_property_unpack(sky_property *property, FILE *file)
{
    int rc;
    size_t sz;
    bstring key = NULL;
    check(property != NULL, "Message required");
    check(file != NULL, "File stream required");

    // Map
    uint32_t map_length = minipack_fread_map(file, &sz);
    check(sz > 0, "Unable to read map");
    
    // Map items
    uint32_t i;
    for(i=0; i<map_length; i++) {
        rc = sky_minipack_fread_bstring(file, &key);
        check(rc == 0, "Unable to read map key");
        
        if(biseqcstr(key, "id")) {
            property->id = (sky_property_id_t)minipack_fread_int(file, &sz);
            check(sz > 0, "Unable to read property id");
        }
        else if(biseqcstr(key, "type")) {
            property->type = (sky_property_type_e)minipack_fread_uint(file, &sz);
            check(sz > 0, "Unable to read property type");
        }
        else if(biseqcstr(key, "dataType")) {
            rc = sky_minipack_fread_bstring(file, &property->data_type);
            check(rc == 0, "Unable to read property data type");
        }
        else if(biseqcstr(key, "name")) {
            rc = sky_minipack_fread_bstring(file, &property->name);
            check(rc == 0, "Unable to read property id");
        }
        
        bdestroy(key);
    }
    
    rc = sky_property_update_type(property);
    check(rc == 0, "Unable to update property type");
    
    return 0;

error:
    bdestroy(key);
    return -1;
}


//--------------------------------------
// Type
//--------------------------------------

// Updates the property type. If the id is greater than 0 then it is an object
// property. If it is less than zero it is an action property. If it is equal
// to zero then just leave the type as-is.
//
// property - The property.
//
// Returns 0 if successful, otherwise returns -1.
int sky_property_update_type(sky_property *property)
{
    check(property != NULL, "Property required");
    
    // Positive property id indicates an object property.
    if(property->id > 0) {
        property->type = SKY_PROPERTY_TYPE_OBJECT;
    }
    // Negative property id indicates an action property.
    else if(property->id < 0) {
        property->type = SKY_PROPERTY_TYPE_ACTION;
    }

    return 0;

error:
    return -1;
}


//--------------------------------------
// Data Type
//--------------------------------------

// Retrieves a reference to a standardized bstring that represents the name
// of the data type.
//
// type_name - The name of the type.
// ret       - A pointer to where the standardized type name should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int sky_property_get_standard_data_type_name(bstring type_name, bstring *ret)
{
    check(type_name != NULL, "Type name required");
    check(ret != NULL, "Return pointer required");

    // Check against standard types.
    if(biseq(&SKY_DATA_TYPE_INT, type_name)) {
        *ret = &SKY_DATA_TYPE_INT;
    }
    else if(biseq(&SKY_DATA_TYPE_FLOAT, type_name)) {
        *ret = &SKY_DATA_TYPE_FLOAT;
    }
    else if(biseq(&SKY_DATA_TYPE_BOOLEAN, type_name)) {
        *ret = &SKY_DATA_TYPE_BOOLEAN;
    }
    else if(biseq(&SKY_DATA_TYPE_STRING, type_name)) {
        *ret = &SKY_DATA_TYPE_STRING;
    }
    // If this is not a standard type then return the name that came in.
    else {
        sentinel("Type is not a standard type: %s", bdata(type_name));
    }

    return 0;

error:
    return -1;
}

