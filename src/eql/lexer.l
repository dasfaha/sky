%{
#include <stdlib.h>
#include "../bstring.h"
#include "ast.h"
#include "array.h"
#include "parser.h"
#define SAVE_STRING yylval->string = bfromcstr(yytext)
#define SAVE_INT yylval->int_value = atoll(yytext)
#define SAVE_FLOAT yylval->float_value = atof(yytext)
#define TOKEN(t) (yylval->token = t)
%}

%option noyywrap
%option reentrant
%option bison-bridge


%%

"class"                 return TOKEN(TCLASS);
"function"              return TOKEN(TFUNCTION);
[ \t\n]                 ;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_STRING; return TIDENTIFIER;
[0-9]+\.[0-9]+          SAVE_FLOAT; return TFLOAT;
[0-9]+                  SAVE_INT; return TINT;
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
";"                     return TOKEN(TSEMICOLON);
":"                     return TOKEN(TCOLON);
","                     return TOKEN(TCOMMA);
.                       printf("Unknown token!\n"); yyterminate();

%%