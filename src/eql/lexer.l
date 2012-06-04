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

%x COMMENT
%x ML_COMMENT

%%

<INITIAL>"//"           BEGIN(COMMENT);
<COMMENT>"\n"           BEGIN(INITIAL); yylineno++;
<COMMENT>[^\n]+

<INITIAL>"/*"           BEGIN(ML_COMMENT);
<ML_COMMENT>"*/"        BEGIN(INITIAL);
<ML_COMMENT>[^*\n]+
<ML_COMMENT>\n          yylineno++;

"class"                 return TOKEN(TCLASS);
"private"               return TOKEN(TPRIVATE);
"public"                return TOKEN(TPUBLIC);
[ \t\n]                 ;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_STRING; return TIDENTIFIER;
[0-9]+\.[0-9]+          SAVE_FLOAT; return TFLOAT;
[0-9]+                  SAVE_INT; return TINT;
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"["                     return TOKEN(TLBRACKET);
"]"                     return TOKEN(TRBRACKET);
"'"                     return TOKEN(TQUOTE);
"\""                    return TOKEN(TDBLQUOTE);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
";"                     return TOKEN(TSEMICOLON);
":"                     return TOKEN(TCOLON);
","                     return TOKEN(TCOMMA);
"="                     return TOKEN(TEQUALS);
.                       printf("Unknown token!\n"); yyterminate();

%%