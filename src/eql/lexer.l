%{
#include "../bstring.h"
#include "ast.h"
#include "parser.h"
#define SAVE_STRING yylval->string = bfromcstr(yytext)
#define SAVE_INT yylval->int_vlue = atolld(yytext)
#define TOKEN(t) (yylval->token = t)
%}

%option noyywrap
%option reentrant
%option bison-bridge


%%

[ \t\n]                 ;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_STRING; return TIDENTIFIER;
[0-9]*                  SAVE_INT; return TINT;
.                       printf("Unknown token!\n"); yyterminate();

%%