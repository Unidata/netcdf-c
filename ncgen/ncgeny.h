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
     NC_UNLIMITED_K = 258,
     CHAR_K = 259,
     BYTE_K = 260,
     SHORT_K = 261,
     INT_K = 262,
     FLOAT_K = 263,
     DOUBLE_K = 264,
     UBYTE_K = 265,
     USHORT_K = 266,
     UINT_K = 267,
     INT64_K = 268,
     UINT64_K = 269,
     STRING_K = 270,
     IDENT = 271,
     TERMSTRING = 272,
     CHAR_CONST = 273,
     BYTE_CONST = 274,
     SHORT_CONST = 275,
     INT_CONST = 276,
     INT64_CONST = 277,
     UBYTE_CONST = 278,
     USHORT_CONST = 279,
     UINT_CONST = 280,
     UINT64_CONST = 281,
     FLOAT_CONST = 282,
     DOUBLE_CONST = 283,
     DIMENSIONS = 284,
     VARIABLES = 285,
     NETCDF = 286,
     DATA = 287,
     TYPES = 288,
     COMPOUND = 289,
     ENUM = 290,
     OPAQUE_ = 291,
     OPAQUESTRING = 292,
     GROUP = 293,
     PATH = 294,
     FILLMARKER = 295,
     NIL = 296,
     _FILLVALUE = 297,
     _FORMAT = 298,
     _STORAGE = 299,
     _CHUNKSIZES = 300,
     _DEFLATELEVEL = 301,
     _SHUFFLE = 302,
     _ENDIANNESS = 303,
     _NOFILL = 304,
     _FLETCHER32 = 305,
     _NCPROPS = 306,
     _ISNETCDF4 = 307,
     _SUPERBLOCK = 308,
     _FILTER = 309,
     DATASETID = 310
   };
#endif
/* Tokens.  */
#define NC_UNLIMITED_K 258
#define CHAR_K 259
#define BYTE_K 260
#define SHORT_K 261
#define INT_K 262
#define FLOAT_K 263
#define DOUBLE_K 264
#define UBYTE_K 265
#define USHORT_K 266
#define UINT_K 267
#define INT64_K 268
#define UINT64_K 269
#define STRING_K 270
#define IDENT 271
#define TERMSTRING 272
#define CHAR_CONST 273
#define BYTE_CONST 274
#define SHORT_CONST 275
#define INT_CONST 276
#define INT64_CONST 277
#define UBYTE_CONST 278
#define USHORT_CONST 279
#define UINT_CONST 280
#define UINT64_CONST 281
#define FLOAT_CONST 282
#define DOUBLE_CONST 283
#define DIMENSIONS 284
#define VARIABLES 285
#define NETCDF 286
#define DATA 287
#define TYPES 288
#define COMPOUND 289
#define ENUM 290
#define OPAQUE_ 291
#define OPAQUESTRING 292
#define GROUP 293
#define PATH 294
#define FILLMARKER 295
#define NIL 296
#define _FILLVALUE 297
#define _FORMAT 298
#define _STORAGE 299
#define _CHUNKSIZES 300
#define _DEFLATELEVEL 301
#define _SHUFFLE 302
#define _ENDIANNESS 303
#define _NOFILL 304
#define _FLETCHER32 305
#define _NCPROPS 306
#define _ISNETCDF4 307
#define _SUPERBLOCK 308
#define _FILTER 309
#define DATASETID 310




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 142 "ncgen.y"
{
Symbol* sym;
unsigned long  size; /* allow for zero size to indicate e.g. UNLIMITED*/
long           mark; /* track indices into the sequence*/
int            nctype; /* for tracking attribute list type*/
Datalist*      datalist;
NCConstant     constant;
}
/* Line 1529 of yacc.c.  */
#line 168 "ncgeny.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE ncglval;

