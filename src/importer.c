#include <stdlib.h>

#include "jsmn/jsmn.h"
#include "importer.h"
#include "dbg.h"


//==============================================================================
//
// Forward Declarations
//
//==============================================================================

int sky_importer_tokenize(sky_importer *importer, bstring source,
    jsmntok_t **tokens);

int sky_importer_parse(sky_importer *importer, FILE *file);

int sky_importer_process(sky_importer *importer, bstring source,
    jsmntok_t *tokens);

int sky_importer_process_table(sky_importer *importer, bstring source,
    jsmntok_t *tokens, uint32_t *index);

int sky_importer_process_actions(sky_importer *importer, bstring source,
    jsmntok_t *tokens, uint32_t *index);

int sky_importer_process_action(sky_importer *importer, bstring source,
    jsmntok_t *tokens, uint32_t *index);

int sky_importer_process_properties(sky_importer *importer, bstring source,
    jsmntok_t *tokens, uint32_t *index);

int sky_importer_process_property(sky_importer *importer, bstring source,
    jsmntok_t *tokens, uint32_t *index);


bool sky_importer_tokstr_equal(bstring source, jsmntok_t *token,
    const char *str);

int32_t sky_importer_token_parse_int(bstring source, jsmntok_t *token);

bstring sky_importer_token_parse_bstring(bstring source, jsmntok_t *token);


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

// Creates an importer.
//
// Returns an importer.
sky_importer *sky_importer_create()
{
    sky_importer *importer = calloc(1, sizeof(sky_importer));
    check_mem(importer);
    return importer;

error:
    sky_importer_free(importer);
    return NULL;
}

// Frees an importer.
//
// Returns nothing.
void sky_importer_free(sky_importer *importer)
{
    if(importer) {
        if(importer->path) bdestroy(importer->path);
        importer->path = NULL;
        
        free(importer);
    }
}

//--------------------------------------
// Path Management
//--------------------------------------

// Sets the path that the importer will import the data to. This can be the
// path of an existing table or to where a table should be created.
//
// importer - The importer.
// path     - The path.
//
// Returns 0 if successful, otherwise returns -1.
int sky_importer_set_path(sky_importer *importer, bstring path)
{
    check(importer != NULL, "Importer required");
    
    if(importer->path) bdestroy(importer->path);
    importer->path = bstrcpy(path);
    if(path) check_mem(importer->path);

    return 0;
    
error:
    return -1;
}

//--------------------------------------
// Import
//--------------------------------------

// Imports a JSON-formatted data stream. The data stream contains table
// information such as properties, actions and block size followed by a series
// of events.
//
// importer - The importer.
// file     - The data stream.
//
// Returns 0 if successful, otherwise returns -1.
int sky_importer_import(sky_importer *importer, FILE *file)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(file != NULL, "File stream required");
    
    // Process json into table structure and events.
    rc = sky_importer_parse(importer, file);
    check(rc == 0, "Unable to process import file");

    // TODO: Create table if a structure exists.
    // TODO: Create events.

    return 0;
    
error:
    return -1;
}


//--------------------------------------
// Parsing
//--------------------------------------

// Parses a JSON document into an array of tokens.
//
// importer - The importer.
// source   - The import file contents.
// tokens   - A pointer to where the tokens should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int sky_importer_tokenize(sky_importer *importer, bstring source,
                          jsmntok_t **tokens)
{
    check(importer != NULL, "Importer required");
    check(source != NULL, "File source required");
    check(tokens != NULL, "Tokens return pointer required");
    
    // Initialize return values.
    *tokens = NULL;
    
    // Create JSON parser.
    jsmn_parser parser;
    jsmn_init(&parser);

    // Parse tokens until we're done.
    uint32_t token_count = 1024;
    jsmnerr_t ret;
    do {
        *tokens = realloc(*tokens, token_count * sizeof(jsmntok_t));
        ret = jsmn_parse(&parser, bdata(source), *tokens, token_count);
        check(ret != JSMN_ERROR_INVAL, "Invalid json token");
        check(ret != JSMN_ERROR_PART, "Unexpected end in json data");
        
        // If we didn't have enough tokens then reallocate and try again.
        if(ret == JSMN_ERROR_NOMEM) token_count *= 2;
    } while(ret != JSMN_SUCCESS);

    return 0;

error:
    free(*tokens);
    *tokens = NULL;
    return -1;
}

// Parses a json import file into an importer structure.
//
// importer    - The importer.
// file        - The data stream.
//
// Returns 0 if successful, otherwise returns -1.
int sky_importer_parse(sky_importer *importer, FILE *file)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(file != NULL, "File stream required");

    // Read in entire stream.
    bstring source = bread((bNread)fread, file); check_mem(source);

    // Read json into tokens.
    jsmntok_t *tokens = NULL;
    rc = sky_importer_tokenize(importer, source, &tokens);
    check(rc == 0, "Unable to tokenize import file");

    // Process the tokens.
    rc = sky_importer_process(importer, source, tokens);
    check(rc == 0, "Unable to parse import root")

    bdestroy(source);
    free(tokens);
    return 0;

error:
    bdestroy(source);
    free(tokens);
    return -1;
}


//--------------------------------------
// Processing
//--------------------------------------

// Processes the JSON tokens.
//
// importer - The importer.
// source   - The JSON source text.
// tokens   - The tokens.
//
// Returns 0 if successful, otherwise returns -1.
int sky_importer_process(sky_importer *importer, bstring source,
                         jsmntok_t *tokens)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");

    // Setup index to track current token and root token.
    uint32_t index = 1;
    jsmntok_t *root_token = &tokens[0];
    
    // Process over child tokens.
    int32_t i;
    for(i=0; i<root_token->size; i++) {
        jsmntok_t *token = &tokens[index];
        index++;
        
        if(sky_importer_tokstr_equal(source, token, "table")) {
            rc = sky_importer_process_table(importer, source, tokens, &index);
            check(rc == 0, "Unable to process table import");
        }
    }
    
    return 0;

error:
    return -1;
}

int sky_importer_process_table(sky_importer *importer, bstring source,
                               jsmntok_t *tokens, uint32_t *index)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");
    check(index != NULL, "Token index required");

    jsmntok_t *table_token = &tokens[*index];
    (*index)++;
    
    // Initialize import table.
    importer->table = sky_table_create(); check_mem(importer->table);
    importer->table->path = bstrcpy(importer->path);
    
    // Process over child tokens.
    int32_t i;
    for(i=0; i<(table_token->size/2); i++) {
        jsmntok_t *token = &tokens[*index];
        (*index)++;
        
        if(sky_importer_tokstr_equal(source, token, "blockSize")) {
            importer->table->default_block_size = (uint32_t)sky_importer_token_parse_int(source, &tokens[(*index)++]);
        }
        else if(sky_importer_tokstr_equal(source, token, "actions")) {
            rc = sky_importer_process_actions(importer, source, tokens, index);
            check(rc == 0, "Unable to process actions import");
        }
        else if(sky_importer_tokstr_equal(source, token, "properties")) {
            rc = sky_importer_process_properties(importer, source, tokens, index);
            check(rc == 0, "Unable to process properties import");
        }
        //else {
        //    sentinel("Invalid token at char %d", tokens[*index].start);
        //}
    }
    
    return 0;

error:
    return -1;
}

int sky_importer_process_actions(sky_importer *importer, bstring source,
                                 jsmntok_t *tokens, uint32_t *index)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");
    check(index != NULL, "Token index required");

    jsmntok_t *actions_token = &tokens[*index];
    (*index)++;
    
    // Process over each action.
    int32_t i;
    for(i=0; i<actions_token->size; i++) {
        rc = sky_importer_process_action(importer, source, tokens, index);
        check(rc == 0, "Unable to process actions import");
    }
    
    return 0;

error:
    return -1;
}

int sky_importer_process_action(sky_importer *importer, bstring source,
                                jsmntok_t *tokens, uint32_t *index)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");
    check(index != NULL, "Token index required");

    jsmntok_t *action_token = &tokens[*index];
    (*index)++;
    
    // Create the action object.
    sky_action *action = sky_action_create(); check_mem(action);
    
    // Process over child tokens.
    int32_t i;
    for(i=0; i<(action_token->size/2); i++) {
        jsmntok_t *token = &tokens[*index];
        (*index)++;
        
        if(sky_importer_tokstr_equal(source, token, "name")) {
            action->name = sky_importer_token_parse_bstring(source, &tokens[*index]);
        }
        else {
            sentinel("Invalid token at char %d", tokens[*index].start);
        }
        (*index)++;
    }
    
    // Add action.
    if(!importer->table->opened) {
        check(sky_table_open(importer->table) == 0, "Unable to open table");
    }
    rc = sky_action_file_add_action(importer->table->action_file, action);
    check(rc == 0, "Unable to add action: %s", bdata(action->name));
    
    return 0;

error:
    return -1;
}

int sky_importer_process_properties(sky_importer *importer, bstring source,
                                    jsmntok_t *tokens, uint32_t *index)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");
    check(index != NULL, "Token index required");

    jsmntok_t *properties_token = &tokens[*index];
    (*index)++;
    
    // Process over each property.
    int32_t i;
    for(i=0; i<properties_token->size; i++) {
        rc = sky_importer_process_property(importer, source, tokens, index);
        check(rc == 0, "Unable to process properties import");
    }
    
    return 0;

error:
    return -1;
}

int sky_importer_process_property(sky_importer *importer, bstring source,
                                  jsmntok_t *tokens, uint32_t *index)
{
    int rc;
    check(importer != NULL, "Importer required");
    check(source != NULL, "Source required");
    check(tokens != NULL, "Tokens required");
    check(index != NULL, "Token index required");

    jsmntok_t *property_token = &tokens[*index];
    (*index)++;
    
    // Create the property object.
    sky_property *property = sky_property_create(); check_mem(property);
    
    // Process over child tokens.
    int32_t i;
    for(i=0; i<(property_token->size/2); i++) {
        jsmntok_t *token = &tokens[*index];
        (*index)++;
        
        if(sky_importer_tokstr_equal(source, token, "type")) {
            bstring type = sky_importer_token_parse_bstring(source, &tokens[*index]);
            property->type = biseqcstr(type, "action") == 1 ? SKY_PROPERTY_TYPE_ACTION : SKY_PROPERTY_TYPE_OBJECT;
            bdestroy(type);
        }
        else if(sky_importer_tokstr_equal(source, token, "dataType")) {
            property->data_type = sky_importer_token_parse_bstring(source, &tokens[*index]);
        }
        else if(sky_importer_tokstr_equal(source, token, "name")) {
            property->name = sky_importer_token_parse_bstring(source, &tokens[*index]);
        }
        else {
            sentinel("Invalid token at char %d", tokens[*index].start);
        }
        
        (*index)++;
    }
    
    // Add property.
    if(!importer->table->opened) {
        check(sky_table_open(importer->table) == 0, "Unable to open table");
    }
    rc = sky_property_file_add_property(importer->table->property_file, property);
    check(rc == 0, "Unable to add property: %s", bdata(property->name));
    
    return 0;

error:
    return -1;
}


//--------------------------------------
// Utility
//--------------------------------------

// Compares a token string to another string.
//
// source - The import file contents.
// token  - The token.
// str    - The string to compare with.
//
// Returns 0 if successful, otherwise returns -1.
bool sky_importer_tokstr_equal(bstring source, jsmntok_t *token,
                               const char *str)
{
    char *source_string = bdata(source);
    int toklen = token->end - token->start;
    if(toklen > 0) {
        return (strncmp(str, &source_string[token->start], toklen) == 0);
    }
    else {
        return false;
    }
}

int32_t sky_importer_token_parse_int(bstring source, jsmntok_t *token)
{
    if(token->type == JSMN_PRIMITIVE) {
        int toklen = token->end - token->start;
        char str[toklen+1];
        char *source_string = bdata(source);
        strncpy(str, &source_string[token->start], toklen);
        str[toklen] = '\x0';
        return (int32_t)atoi(str);
    }
    
    return 0;
}

bstring sky_importer_token_parse_bstring(bstring source, jsmntok_t *token)
{
    if(token->type == JSMN_STRING) {
        int toklen = token->end - token->start;
        return bmidstr(source, token->start, toklen);
    }
    
    return NULL;
}