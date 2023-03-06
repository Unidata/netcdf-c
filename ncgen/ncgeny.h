/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_NCG_NCGEN_TAB_H_INCLUDED
# define YY_NCG_NCGEN_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int ncgdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    NC_UNLIMITED_K = 258,          /* NC_UNLIMITED_K  */
    CHAR_K = 259,                  /* CHAR_K  */
    BYTE_K = 260,                  /* BYTE_K  */
    SHORT_K = 261,                 /* SHORT_K  */
    INT_K = 262,                   /* INT_K  */
    FLOAT_K = 263,                 /* FLOAT_K  */
    DOUBLE_K = 264,                /* DOUBLE_K  */
    UBYTE_K = 265,                 /* UBYTE_K  */
    USHORT_K = 266,                /* USHORT_K  */
    UINT_K = 267,                  /* UINT_K  */
    INT64_K = 268,                 /* INT64_K  */
    UINT64_K = 269,                /* UINT64_K  */
    STRING_K = 270,                /* STRING_K  */
    IDENT = 271,                   /* IDENT  */
    TERMSTRING = 272,              /* TERMSTRING  */
    CHAR_CONST = 273,              /* CHAR_CONST  */
    BYTE_CONST = 274,              /* BYTE_CONST  */
    SHORT_CONST = 275,             /* SHORT_CONST  */
    INT_CONST = 276,               /* INT_CONST  */
    INT64_CONST = 277,             /* INT64_CONST  */
    UBYTE_CONST = 278,             /* UBYTE_CONST  */
    USHORT_CONST = 279,            /* USHORT_CONST  */
    UINT_CONST = 280,              /* UINT_CONST  */
    UINT64_CONST = 281,            /* UINT64_CONST  */
    FLOAT_CONST = 282,             /* FLOAT_CONST  */
    DOUBLE_CONST = 283,            /* DOUBLE_CONST  */
    DIMENSIONS = 284,              /* DIMENSIONS  */
    VARIABLES = 285,               /* VARIABLES  */
    NETCDF = 286,                  /* NETCDF  */
    DATA = 287,                    /* DATA  */
    TYPES = 288,                   /* TYPES  */
    COMPOUND = 289,                /* COMPOUND  */
    ENUM = 290,                    /* ENUM  */
    OPAQUE_ = 291,                 /* OPAQUE_  */
    OPAQUESTRING = 292,            /* OPAQUESTRING  */
    GROUP = 293,                   /* GROUP  */
    PATH = 294,                    /* PATH  */
    FILLMARKER = 295,              /* FILLMARKER  */
    NIL = 296,                     /* NIL  */
    _FILLVALUE = 297,              /* _FILLVALUE  */
    _FORMAT = 298,                 /* _FORMAT  */
    _STORAGE = 299,                /* _STORAGE  */
    _CHUNKSIZES = 300,             /* _CHUNKSIZES  */
    _DEFLATELEVEL = 301,           /* _DEFLATELEVEL  */
    _SHUFFLE = 302,                /* _SHUFFLE  */
    _ENDIANNESS = 303,             /* _ENDIANNESS  */
    _NOFILL = 304,                 /* _NOFILL  */
    _FLETCHER32 = 305,             /* _FLETCHER32  */
    _NCPROPS = 306,                /* _NCPROPS  */
    _ISNETCDF4 = 307,              /* _ISNETCDF4  */
    _SUPERBLOCK = 308,             /* _SUPERBLOCK  */
    _FILTER = 309,                 /* _FILTER  */
    _CODECS = 310,                 /* _CODECS  */
    _QUANTIZEBG = 311,             /* _QUANTIZEBG  */
    _QUANTIZEGBR = 312,            /* _QUANTIZEGBR  */
    _QUANTIZEBR = 313,             /* _QUANTIZEBR  */
    DATASETID = 314                /* DATASETID  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 156 "ncgen.y"

Symbol* sym;
unsigned long  size; /* allow for zero size to indicate e.g. UNLIMITED*/
long           mark; /* track indices into the sequence*/
int            nctype; /* for tracking attribute list type*/
Datalist*      datalist;
NCConstant*    constant;

#line 132 "ncgeny.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE ncglval;

int ncgparse (void);

#endif /* !YY_NCG_NCGEN_TAB_H_INCLUDED  */
