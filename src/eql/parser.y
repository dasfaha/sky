%{
    #include "stdio.h"
    #include "ast.h"
    #include "parser.h"
    #include "lexer.h"
    #include "../dbg.h"
    eql_ast_node *root;
    extern int yylex();
    void yyerror(void *scanner, const char *s) { printf("ERROR: %s\n", s); }
%}

%debug
%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%code provides {
    int eql_parse(bstring name, bstring text, eql_ast_node **module);
}

%union {
    bstring string;
    int64_t int_value;
    double float_value;
    eql_ast_node *node;
    int token;
}

%token <int_value> TINT
%token <float_value> TFLOAT

%type <node> block expr
%type <node> number int_literal float_literal

%start module

%%

module : /* empty */
        | block     { root->module.block = $1; }
;

block   : expr      { eql_ast_block_create(&$1, 1, &$$); };

expr    : number
;

number  : float_literal
        | int_literal
;

int_literal  : TINT { eql_ast_int_literal_create($1, &$$); };

float_literal  : TFLOAT { eql_ast_float_literal_create($1, &$$); };

%%


//==============================================================================
//
// Functions
//
//==============================================================================

// Parses a string that contains EQL program text.
//
// name   - The name of the module.
// text   - The text for the EQL module.
// module - The pointer where the module AST should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_parse(bstring name, bstring text, eql_ast_node **module)
{
    // yydebug = 1;
    
    // Setup module.
    int rc = eql_ast_module_create(name, NULL, 0, NULL, &root);
    check(rc == 0, "Unable to create module");
    
    // Parse using Bison.
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE buffer = yy_scan_string(bdata(text), scanner);
    rc = yyparse(scanner);
    yy_delete_buffer(buffer, scanner);
    yylex_destroy(scanner);
    check(rc == 0, "EQL Syntax error");

    // Return module to caller.
    *module = root;

    return 0;

error:
    return -1;
}
