%{
    #include "stdbool.h"
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
    bool boolean_value;
    eql_ast_access_e access;
    eql_ast_node *node;
    eql_array *array;
    struct {
        eql_ast_node *condition;
        eql_ast_node *block;
    } if_block;
    struct {
        eql_array *conditions;
        eql_array *blocks;
    } if_blocks;
    int token;
}

%token <string> TIDENTIFIER TSTRING
%token <int_value> TINT
%token <float_value> TFLOAT
%token <boolean_value> TTRUE TFALSE
%token <token> TCLASS
%token <token> TPUBLIC TPRIVATE TRETURN
%token <token> TIF TELSE
%token <token> TFOR TEACH TIN
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TLBRACKET TRBRACKET
%token <token> TQUOTE TDBLQUOTE
%token <token> TSEMICOLON TCOLON TCOMMA
%token <token> TPLUS TMINUS TMUL TDIV TASSIGN
%token <token> TEQUALS
%token <token> TDOT

%type <string> string
%type <node> block stmt expr
%type <node> if_stmt else_block for_each_stmt
%type <node> var_ref var_decl
%type <node> class method property
%type <node> function farg fcall
%type <node> number literal int_literal float_literal boolean_literal
%type <node> metadata metadata_item
%type <array> stmts fcall_args fargs class_members metadatas metadata_items
%type <if_block> if_block else_if_block
%type <if_blocks> else_if_blocks
%type <access> access

%left TASSIGN
%left TEQUALS
%left TPLUS TMINUS
%left TMUL TDIV
%left TDOT

%start module

%%

module  : /* empty */
        | module class     { eql_ast_module_add_class(root, $2); }
        | module stmt      { eql_ast_block_add_expr(root->module.main_function->function.body, $2); }
;

block   : /* empty */ { $$ = NULL; }
        | stmts       { eql_ast_block_create(NULL, (eql_ast_node**)$1->elements, $1->length, &$$); eql_array_free($1); };

stmts   : stmt       { $$ = eql_array_create(); eql_array_push($$, $1); }
        | stmts stmt { eql_array_push($1, $2); }

stmt    : expr TSEMICOLON
        | var_decl TSEMICOLON
        | TRETURN expr TSEMICOLON { eql_ast_freturn_create($2, &$$); }
        | TRETURN TSEMICOLON { eql_ast_freturn_create(NULL, &$$); }
        | var_ref TASSIGN expr TSEMICOLON { eql_ast_var_assign_create($1, $3, &$$); }
        | if_stmt
        | for_each_stmt
;

expr    : expr TPLUS expr     { eql_ast_binary_expr_create(EQL_BINOP_PLUS, $1, $3, &$$); }
        | expr TMINUS expr    { eql_ast_binary_expr_create(EQL_BINOP_MINUS, $1, $3, &$$); }
        | expr TMUL expr      { eql_ast_binary_expr_create(EQL_BINOP_MUL, $1, $3, &$$); }
        | expr TDIV expr      { eql_ast_binary_expr_create(EQL_BINOP_DIV, $1, $3, &$$); }
        | expr TEQUALS expr   { eql_ast_binary_expr_create(EQL_BINOP_EQUALS, $1, $3, &$$); }
        | literal
        | var_ref
        | fcall
        | TLPAREN expr TRPAREN { $$ = $2; }
;

var_ref : TIDENTIFIER { eql_ast_var_ref_create($1, &$$); bdestroy($1); }
        | var_ref TDOT TIDENTIFIER { eql_ast_staccess_create($1, $3, &$$); bdestroy($3); }
;

var_decl : TIDENTIFIER TIDENTIFIER { eql_ast_var_decl_create($1, $2, &$$); bdestroy($1); bdestroy($2); };

string : TSTRING;

literal : boolean_literal
        | number
;

number  : float_literal
        | int_literal
;

int_literal  : TINT { eql_ast_int_literal_create($1, &$$); };

float_literal  : TFLOAT { eql_ast_float_literal_create($1, &$$); };

boolean_literal  : TTRUE  { eql_ast_boolean_literal_create(true, &$$); }
                 | TFALSE { eql_ast_boolean_literal_create(false, &$$); }
;

fcall : TIDENTIFIER TLPAREN fcall_args TRPAREN
        {
            eql_ast_fcall_create($1, (eql_ast_node**)$3->elements, $3->length, &$$);
            bdestroy($1);
            eql_array_free($3);
        }
;

fcall_args  : /* empty */            { $$ = eql_array_create(); }
            | expr                   { $$ = eql_array_create(); eql_array_push($$, $1); }
            | fcall_args TCOMMA expr { eql_array_push($1, $3); }
;

function : TIDENTIFIER TIDENTIFIER TLPAREN fargs TRPAREN TLBRACE block TRBRACE
           {
               eql_ast_function_create($2, $1, (eql_ast_node **)$4->elements, $4->length, $7, &$$);
               bdestroy($1);
               bdestroy($2);
               eql_array_free($4);
           }
;

fargs  : /* empty */        { $$ = eql_array_create(); }
       | farg               { $$ = eql_array_create(); eql_array_push($$, $1); }
       | fargs TCOMMA farg  { eql_array_push($1, $3); }
;

farg   : var_decl  { eql_ast_farg_create($1, &$$); };

if_stmt : if_block else_if_blocks else_block
          {
              eql_ast_if_stmt_create(&$$);
              eql_ast_if_stmt_add_block($$, $1.condition, $1.block);
              eql_ast_if_stmt_add_blocks($$, (eql_ast_node**)$2.conditions->elements, (eql_ast_node**)$2.blocks->elements, $2.blocks->length);
              eql_ast_if_stmt_set_else_block($$, $3);
              eql_array_free($2.conditions);
              eql_array_free($2.blocks);
          }
;

if_block : TIF TLPAREN expr TRPAREN TLBRACE block TRBRACE { $$.condition = $3; $$.block = $6; };

else_if_blocks : /* empty */                     { $$.conditions = eql_array_create(); $$.blocks = eql_array_create(); }
               | else_if_blocks else_if_block    { eql_array_push($1.conditions, $2.condition); eql_array_push($1.blocks, $2.block); }
;

else_if_block : TELSE if_block                 { $$ = $2; }

else_block : /* empty */                 { $$ = NULL; }
           | TELSE TLBRACE block TRBRACE { $$ = $3; }
;

for_each_stmt : TFOR TEACH TLPAREN var_decl TIN expr TRPAREN TLBRACE block TRBRACE
                {
                    eql_ast_for_each_stmt_create($4, $6, $9, &$$);
                }
;

access : TPUBLIC    { $$ = EQL_ACCESS_PUBLIC; }
       | TPRIVATE   { $$ = EQL_ACCESS_PRIVATE; }
;

class : metadatas TCLASS TIDENTIFIER TLBRACE class_members TRBRACE
        {
            eql_ast_class_create($3, NULL, 0, NULL, 0, &$$);
            eql_ast_class_add_members($$, (eql_ast_node**)$5->elements, $5->length);
            eql_ast_class_add_metadatas($$, (eql_ast_node**)$1->elements, $1->length);
            bdestroy($3);
            free($1);
            free($5);
        };

class_members : /* empty */             { $$ = eql_array_create(); }
              | class_members method    { eql_array_push($$, $2); }
              | class_members property  { eql_array_push($$, $2); }
;

method  : access function { eql_ast_method_create($1, $2, &$$); };

property  : access var_decl TSEMICOLON { eql_ast_property_create($1, $2, &$$); };
        
metadatas : /* empty */        { $$ = eql_array_create(); }
          | metadatas metadata { eql_array_push($$, $2); }
;

metadata  : TLBRACKET TIDENTIFIER TRBRACKET { eql_ast_metadata_create($2, NULL, 0, &$$); bdestroy($2); }
          | TLBRACKET TIDENTIFIER TLPAREN metadata_items TRPAREN TRBRACKET { eql_ast_metadata_create($2, (eql_ast_node**)$4->elements, $4->length, &$$); bdestroy($2); free($4); }
;
    
metadata_items : /* empty */                         { $$ = eql_array_create(); }
               | metadata_item                       { $$ = eql_array_create(); eql_array_push($$, $1); }
               | metadata_items TCOMMA metadata_item { eql_array_push($1, $3); }
;

metadata_item   : TIDENTIFIER TASSIGN string { eql_ast_metadata_item_create($1, $3, &$$); bdestroy($1); bdestroy($3); };

%%


//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring mainFunctionName = bsStatic("main");


//==============================================================================
//
// Functions
//
//==============================================================================

//--------------------------------------
// Parsing
//--------------------------------------

// Parses a string that contains EQL program text.
//
// name   - The name of the module.
// text   - The text for the EQL module.
// module - The pointer where the module AST should be returned.
//
// Returns 0 if successful, otherwise returns -1.
int eql_parse(bstring name, bstring text, eql_ast_node **module)
{
    int rc;
    //yydebug = 1;
    
    // Setup module.
    rc = eql_ast_module_create(name, NULL, 0, NULL, &root);
    check(rc == 0, "Unable to create module");
    
    // Setup main function.
	eql_ast_node *main_block = NULL;
    rc = eql_ast_block_create(NULL, NULL, 0, &main_block);
    check(rc == 0, "Unable to create root block for main function");
    rc = eql_ast_function_create(&mainFunctionName, NULL, NULL, 0, main_block, &root->module.main_function);
    check(rc == 0, "Unable to create main function");
    
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
