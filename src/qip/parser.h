/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */

/* Line 2132 of yacc.c  */
#line 24 "src/parser.y"

    #include "node.h"
    #include "array.h"
    #include "error.h"



/* Line 2132 of yacc.c  */
#line 46 "src/parser.h"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TIDENTIFIER = 258,
     TSTRING = 259,
     TINT = 260,
     TFLOAT = 261,
     TTRUE = 262,
     TFALSE = 263,
     TCLASS = 264,
     TPUBLIC = 265,
     TPRIVATE = 266,
     TRETURN = 267,
     TIF = 268,
     TELSE = 269,
     TFOR = 270,
     TEACH = 271,
     TIN = 272,
     TLPAREN = 273,
     TRPAREN = 274,
     TLBRACE = 275,
     TRBRACE = 276,
     TLBRACKET = 277,
     TRBRACKET = 278,
     TLANGLE = 279,
     TRANGLE = 280,
     TQUOTE = 281,
     TDBLQUOTE = 282,
     TSEMICOLON = 283,
     TCOLON = 284,
     TCOMMA = 285,
     TPLUS = 286,
     TMINUS = 287,
     TMUL = 288,
     TDIV = 289,
     TASSIGN = 290,
     TEQUALS = 291,
     TDOT = 292,
     TSIZEOF = 293,
     TNULL = 294,
     TFUNCTION = 295
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2132 of yacc.c  */
#line 47 "src/parser.y"

    bstring string;
    int64_t int_value;
    double float_value;
    bool boolean_value;
    qip_ast_access_e access;
    qip_ast_node *node;
    qip_array *array;
    struct {
        qip_ast_node *condition;
        qip_ast_node *block;
    } if_block;
    struct {
        qip_array *conditions;
        qip_array *blocks;
    } if_blocks;
    int token;



/* Line 2132 of yacc.c  */
#line 124 "src/parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



/* "%code provides" blocks.  */

/* Line 2132 of yacc.c  */
#line 30 "src/parser.y"

    #ifndef _qip_parser_typedef
    #define _qip_parser_typedef
    typedef struct qip_parser {
        uint32_t error_count;
        qip_error **errors;
    } qip_parser;
    #endif

    qip_parser *qip_parser_create();
    void qip_parser_free(qip_parser *parser);
    int qip_parser_parse(qip_parser *parser, bstring name, bstring text, qip_ast_node **module);
    int qip_parser_add_error(qip_parser *parser, int32_t line_no, bstring message);
    int qip_parser_free_errors(qip_parser *parser);
    int qip_set_pos(qip_ast_node *node, YYLTYPE *loc);



/* Line 2132 of yacc.c  */
#line 171 "src/parser.h"
