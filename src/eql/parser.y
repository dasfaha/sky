%{
    #include "stdio.h"
    #include "ast.h"
    #include "parser.h"
    #include "lexer.h"
    eql_ast_node *root;
    extern int yylex();
    void yyerror(void *scanner, const char *s) { printf("ERROR: %s\n", s); }
%}

%debug
%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%code provides {
    int eql_parse(char *text, eql_ast_node **module);
}

%union {
    bstring string;
    int64_t int_value;
    double float_value;
    eql_ast_node *node;
    int token;
}

%token <int_value> TINT

%type <node> expr

%start module

%%

module : /* empty */
        | expr     { root = $1; }
;

int_literal  : TINT { eql_ast_int_literal_create($1, &$$);};

expr    : int_literal
;

%%


//==============================================================================
//
// Functions
//
//==============================================================================

// Parses a string that contains EQL program text.
//
// text   - The text for the EQL module.
// module - The pointer where the module AST should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_parse(char *text, eql_ast_node **module)
{
    // yydebug = 1;
    
    // Parse using Bison.
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE buffer = yy_scan_string(text, scanner);
    int rc = yyparse(scanner);
    yy_delete_buffer(buffer, scanner);
    yylex_destroy(scanner);
    
    // If parse was successful, return root node.
    if(rc == 0) {
        *module = root;
        return 0;
    }
    // Otherwise return error.
    else {
        return -1;
    }
}
