%{
    #include "stdio.h"
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

%code requires {
    #include "ast.h"
    #include "array.h"
}

%code provides {
    int eql_parse(bstring name, bstring text, eql_ast_node **module);
}

%union {
    bstring string;
    int64_t int_value;
    double float_value;
    eql_ast_node *node;
    eql_array *array;
    int token;
}

%token <string> TIDENTIFIER
%token <int_value> TINT
%token <float_value> TFLOAT
%token <token> TLPAREN TRPAREN TSEMICOLON
%token <token> TPLUS TMINUS TMUL TDIV

%type <node> block stmt expr
%type <node> var_ref
%type <node> number int_literal float_literal
%type <array> stmts

%left TPLUS TMINUS
%left TMUL TDIV

%start module

%%

module : /* empty */
        | block     { root->module.block = $1; }
;

block   : /* empty */
        | stmts      { eql_ast_block_create((eql_ast_node**)$1->elements, $1->length, &$$); eql_array_free($1); };

stmts   : stmt       { $$ = eql_array_create(); eql_array_push($$, $1); }
        | stmts stmt { eql_array_push($1, $2); }

stmt    : expr TSEMICOLON;

expr    : expr TPLUS expr   { eql_ast_binary_expr_create(EQL_BINOP_PLUS, $1, $3, &$$); }
        | expr TMINUS expr  { eql_ast_binary_expr_create(EQL_BINOP_MINUS, $1, $3, &$$); }
        | expr TMUL expr    { eql_ast_binary_expr_create(EQL_BINOP_MUL, $1, $3, &$$); }
        | expr TDIV expr    { eql_ast_binary_expr_create(EQL_BINOP_DIV, $1, $3, &$$); }
        | number
        | var_ref
        | TLPAREN expr TRPAREN { $$ = $2; }
;

var_ref : TIDENTIFIER { eql_ast_var_ref_create($1, &$$); bdestroy($1); };

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
