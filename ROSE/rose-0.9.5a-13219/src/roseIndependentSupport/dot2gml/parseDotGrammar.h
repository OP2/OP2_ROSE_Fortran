/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     UNDS = 259,
     VNULL = 260,
     NAME = 261,
     ID = 262,
     LBRACE = 263,
     RBRACE = 264,
     LBRK = 265,
     RBRK = 266,
     ASSGN = 267,
     LABEL = 268,
     LARROW = 269,
     DIGRAPH = 270,
     COLUMN = 271,
     SHAPE = 272,
     SUBGRAPH = 273,
     COLOR = 274,
     NONE = 275,
     BOTH = 276,
     DIAMOND = 277,
     DIR = 278
   };
#endif
/* Tokens.  */
#define STRING 258
#define UNDS 259
#define VNULL 260
#define NAME 261
#define ID 262
#define LBRACE 263
#define RBRACE 264
#define LBRK 265
#define RBRK 266
#define ASSGN 267
#define LABEL 268
#define LARROW 269
#define DIGRAPH 270
#define COLUMN 271
#define SHAPE 272
#define SUBGRAPH 273
#define COLOR 274
#define NONE 275
#define BOTH 276
#define DIAMOND 277
#define DIR 278




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 23 "../../../../src/roseIndependentSupport/dot2gml/parseDotGrammar.yy"
{
  int iVal;
  char cVal;
  char *sVal;
}
/* Line 1529 of yacc.c.  */
#line 101 "parseDotGrammar.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

