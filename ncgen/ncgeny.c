/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30705

/* Bison version string.  */
#define YYBISON_VERSION "3.7.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         ncgparse
#define yylex           ncglex
#define yyerror         ncgerror
#define yydebug         ncgdebug
#define yynerrs         ncgnerrs
#define yylval          ncglval
#define yychar          ncgchar

/* First part of user prologue.  */
#line 11 "ncgen.y"

/*
static char SccsId[] = "$Id: ncgen.y,v 1.42 2010/05/18 21:32:46 dmh Exp $";
*/
#include        "includes.h"
#include        "netcdf_aux.h"
#include        "ncgeny.h"
#include        "ncgen.h"
#ifdef USE_NETCDF4
#include        "netcdf_filter.h"
#endif

/* Following are in ncdump (for now)*/
/* Need some (unused) definitions to get it to compile */
#define ncatt_t void*
#define ncvar_t void
#include "nctime.h"

#undef GENLIB1

/* parser controls */
#define YY_NO_INPUT 1

/* True if string a equals string b*/
#ifndef NCSTREQ
#define NCSTREQ(a, b)     (*(a) == *(b) && strcmp((a), (b)) == 0)
#endif
#define VLENSIZE  (sizeof(nc_vlen_t))
#define MAXFLOATDIM 4294967295.0

/* mnemonics */
typedef enum Attrkind {ATTRVAR, ATTRGLOBAL, DONTKNOW} Attrkind;

#define ISCONST 1
#define ISLIST 0

typedef nc_vlen_t vlen_t;

/* We retain the old representation of the symbol list
   as a linked list.
*/
List* symlist;

/* Track rootgroup separately*/
Symbol* rootgroup;

/* Track the group sequence */
static List* groupstack;

/* Provide a separate sequence for accumulating values
   during the parse.
*/
static List* stack;

/* track homogeneity of types for data lists*/
static nc_type consttype;

/* Misc. */
static int stackbase;
static int stacklen;
static int count;
static int opaqueid; /* counter for opaque constants*/
static int arrayuid; /* counter for pseudo-array types*/

char* primtypenames[PRIMNO] = {
"nat",
"byte", "char", "short",
"int", "float", "double",
"ubyte", "ushort", "uint",
"int64", "uint64",
"string"
};

static int GLOBAL_SPECIAL = _NCPROPS_FLAG
                            | _ISNETCDF4_FLAG
                            | _SUPERBLOCK_FLAG
                            | _FORMAT_FLAG ;

/*Defined in ncgen.l*/
extern int lineno;              /* line number for error messages */
extern Bytebuffer* lextext;           /* name or string with escapes removed */

extern double double_val;       /* last double value read */
extern float float_val;         /* last float value read */
extern long long int64_val;         /* last int64 value read */
extern int int32_val;             /* last int32 value read */
extern short int16_val;         /* last short value read */
extern unsigned long long uint64_val;         /* last int64 value read */
extern unsigned int uint32_val;             /* last int32 value read */
extern unsigned short uint16_val;         /* last short value read */
extern char char_val;           /* last char value read */
extern signed char byte_val;    /* last byte value read */
extern unsigned char ubyte_val;    /* last byte value read */

/* Track definitions of dims, types, attributes, and vars*/
List* grpdefs;
List* dimdefs;
List* attdefs; /* variable-specific attributes*/
List* gattdefs; /* global attributes only*/
List* xattdefs; /* unknown attributes*/
List* typdefs;
List* vardefs;
List* tmp;

/* Forward */
static NCConstant* makeconstdata(nc_type);
static NCConstant* evaluate(Symbol* fcn, Datalist* arglist);
static NCConstant* makeenumconstref(Symbol*);
static void addtogroup(Symbol*);
static Symbol* currentgroup(void);
static Symbol* createrootgroup(const char*);
static Symbol* creategroup(Symbol*);
static int dupobjectcheck(nc_class,Symbol*);
static void setpathcurrent(Symbol* sym);
static Symbol* makeattribute(Symbol*,Symbol*,Symbol*,Datalist*,Attrkind);
static Symbol* makeprimitivetype(nc_type i);
static Symbol* makespecial(int tag, Symbol* vsym, Symbol* tsym, void* data, int isconst);
static int containsfills(Datalist* list);
static void vercheck(int ncid);
static long long extractint(NCConstant* con);
#ifdef USE_NETCDF4
static int parsefilterflag(const char* sdata0, Specialdata* special);
static int parsecodecsflag(const char* sdata0, Specialdata* special);
static Symbol* identkeyword(const Symbol*);

#ifdef GENDEBUG1
static void printfilters(int nfilters, NC_ParsedFilterSpec** filters);
#endif
#endif

int yylex(void);

#ifndef NO_STDARG
static void yyerror(const char *fmt, ...);
#else
static void yyerror(fmt,va_alist) const char* fmt; va_dcl;
#endif

/* Extern */
extern int lex_init(void);


#line 221 "ncgeny.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "ncgeny.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_NC_UNLIMITED_K = 3,             /* NC_UNLIMITED_K  */
  YYSYMBOL_CHAR_K = 4,                     /* CHAR_K  */
  YYSYMBOL_BYTE_K = 5,                     /* BYTE_K  */
  YYSYMBOL_SHORT_K = 6,                    /* SHORT_K  */
  YYSYMBOL_INT_K = 7,                      /* INT_K  */
  YYSYMBOL_FLOAT_K = 8,                    /* FLOAT_K  */
  YYSYMBOL_DOUBLE_K = 9,                   /* DOUBLE_K  */
  YYSYMBOL_UBYTE_K = 10,                   /* UBYTE_K  */
  YYSYMBOL_USHORT_K = 11,                  /* USHORT_K  */
  YYSYMBOL_UINT_K = 12,                    /* UINT_K  */
  YYSYMBOL_INT64_K = 13,                   /* INT64_K  */
  YYSYMBOL_UINT64_K = 14,                  /* UINT64_K  */
  YYSYMBOL_STRING_K = 15,                  /* STRING_K  */
  YYSYMBOL_IDENT = 16,                     /* IDENT  */
  YYSYMBOL_TERMSTRING = 17,                /* TERMSTRING  */
  YYSYMBOL_CHAR_CONST = 18,                /* CHAR_CONST  */
  YYSYMBOL_BYTE_CONST = 19,                /* BYTE_CONST  */
  YYSYMBOL_SHORT_CONST = 20,               /* SHORT_CONST  */
  YYSYMBOL_INT_CONST = 21,                 /* INT_CONST  */
  YYSYMBOL_INT64_CONST = 22,               /* INT64_CONST  */
  YYSYMBOL_UBYTE_CONST = 23,               /* UBYTE_CONST  */
  YYSYMBOL_USHORT_CONST = 24,              /* USHORT_CONST  */
  YYSYMBOL_UINT_CONST = 25,                /* UINT_CONST  */
  YYSYMBOL_UINT64_CONST = 26,              /* UINT64_CONST  */
  YYSYMBOL_FLOAT_CONST = 27,               /* FLOAT_CONST  */
  YYSYMBOL_DOUBLE_CONST = 28,              /* DOUBLE_CONST  */
  YYSYMBOL_DIMENSIONS = 29,                /* DIMENSIONS  */
  YYSYMBOL_VARIABLES = 30,                 /* VARIABLES  */
  YYSYMBOL_NETCDF = 31,                    /* NETCDF  */
  YYSYMBOL_DATA = 32,                      /* DATA  */
  YYSYMBOL_TYPES = 33,                     /* TYPES  */
  YYSYMBOL_COMPOUND = 34,                  /* COMPOUND  */
  YYSYMBOL_ENUM = 35,                      /* ENUM  */
  YYSYMBOL_OPAQUE_ = 36,                   /* OPAQUE_  */
  YYSYMBOL_OPAQUESTRING = 37,              /* OPAQUESTRING  */
  YYSYMBOL_GROUP = 38,                     /* GROUP  */
  YYSYMBOL_PATH = 39,                      /* PATH  */
  YYSYMBOL_FILLMARKER = 40,                /* FILLMARKER  */
  YYSYMBOL_NIL = 41,                       /* NIL  */
  YYSYMBOL__FILLVALUE = 42,                /* _FILLVALUE  */
  YYSYMBOL__FORMAT = 43,                   /* _FORMAT  */
  YYSYMBOL__STORAGE = 44,                  /* _STORAGE  */
  YYSYMBOL__CHUNKSIZES = 45,               /* _CHUNKSIZES  */
  YYSYMBOL__DEFLATELEVEL = 46,             /* _DEFLATELEVEL  */
  YYSYMBOL__SHUFFLE = 47,                  /* _SHUFFLE  */
  YYSYMBOL__ENDIANNESS = 48,               /* _ENDIANNESS  */
  YYSYMBOL__NOFILL = 49,                   /* _NOFILL  */
  YYSYMBOL__FLETCHER32 = 50,               /* _FLETCHER32  */
  YYSYMBOL__NCPROPS = 51,                  /* _NCPROPS  */
  YYSYMBOL__ISNETCDF4 = 52,                /* _ISNETCDF4  */
  YYSYMBOL__SUPERBLOCK = 53,               /* _SUPERBLOCK  */
  YYSYMBOL__FILTER = 54,                   /* _FILTER  */
  YYSYMBOL__CODECS = 55,                   /* _CODECS  */
  YYSYMBOL__QUANTIZEBG = 56,               /* _QUANTIZEBG  */
  YYSYMBOL__QUANTIZEGBR = 57,              /* _QUANTIZEGBR  */
  YYSYMBOL__QUANTIZEBR = 58,               /* _QUANTIZEBR  */
  YYSYMBOL_DATASETID = 59,                 /* DATASETID  */
  YYSYMBOL_60_ = 60,                       /* '{'  */
  YYSYMBOL_61_ = 61,                       /* '}'  */
  YYSYMBOL_62_ = 62,                       /* ';'  */
  YYSYMBOL_63_ = 63,                       /* ','  */
  YYSYMBOL_64_ = 64,                       /* '='  */
  YYSYMBOL_65_ = 65,                       /* '('  */
  YYSYMBOL_66_ = 66,                       /* ')'  */
  YYSYMBOL_67_ = 67,                       /* '*'  */
  YYSYMBOL_68_ = 68,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 69,                  /* $accept  */
  YYSYMBOL_ncdesc = 70,                    /* ncdesc  */
  YYSYMBOL_datasetid = 71,                 /* datasetid  */
  YYSYMBOL_rootgroup = 72,                 /* rootgroup  */
  YYSYMBOL_groupbody = 73,                 /* groupbody  */
  YYSYMBOL_subgrouplist = 74,              /* subgrouplist  */
  YYSYMBOL_namedgroup = 75,                /* namedgroup  */
  YYSYMBOL_76_1 = 76,                      /* $@1  */
  YYSYMBOL_77_2 = 77,                      /* $@2  */
  YYSYMBOL_typesection = 78,               /* typesection  */
  YYSYMBOL_typedecls = 79,                 /* typedecls  */
  YYSYMBOL_typename = 80,                  /* typename  */
  YYSYMBOL_type_or_attr_decl = 81,         /* type_or_attr_decl  */
  YYSYMBOL_typedecl = 82,                  /* typedecl  */
  YYSYMBOL_optsemicolon = 83,              /* optsemicolon  */
  YYSYMBOL_enumdecl = 84,                  /* enumdecl  */
  YYSYMBOL_enumidlist = 85,                /* enumidlist  */
  YYSYMBOL_enumid = 86,                    /* enumid  */
  YYSYMBOL_opaquedecl = 87,                /* opaquedecl  */
  YYSYMBOL_vlendecl = 88,                  /* vlendecl  */
  YYSYMBOL_compounddecl = 89,              /* compounddecl  */
  YYSYMBOL_fields = 90,                    /* fields  */
  YYSYMBOL_field = 91,                     /* field  */
  YYSYMBOL_primtype = 92,                  /* primtype  */
  YYSYMBOL_dimsection = 93,                /* dimsection  */
  YYSYMBOL_dimdecls = 94,                  /* dimdecls  */
  YYSYMBOL_dim_or_attr_decl = 95,          /* dim_or_attr_decl  */
  YYSYMBOL_dimdeclist = 96,                /* dimdeclist  */
  YYSYMBOL_dimdecl = 97,                   /* dimdecl  */
  YYSYMBOL_dimd = 98,                      /* dimd  */
  YYSYMBOL_vasection = 99,                 /* vasection  */
  YYSYMBOL_vadecls = 100,                  /* vadecls  */
  YYSYMBOL_vadecl_or_attr = 101,           /* vadecl_or_attr  */
  YYSYMBOL_vardecl = 102,                  /* vardecl  */
  YYSYMBOL_varlist = 103,                  /* varlist  */
  YYSYMBOL_varspec = 104,                  /* varspec  */
  YYSYMBOL_dimspec = 105,                  /* dimspec  */
  YYSYMBOL_dimlist = 106,                  /* dimlist  */
  YYSYMBOL_dimref = 107,                   /* dimref  */
  YYSYMBOL_fieldlist = 108,                /* fieldlist  */
  YYSYMBOL_fieldspec = 109,                /* fieldspec  */
  YYSYMBOL_fielddimspec = 110,             /* fielddimspec  */
  YYSYMBOL_fielddimlist = 111,             /* fielddimlist  */
  YYSYMBOL_fielddim = 112,                 /* fielddim  */
  YYSYMBOL_varref = 113,                   /* varref  */
  YYSYMBOL_typeref = 114,                  /* typeref  */
  YYSYMBOL_ambiguous_ref = 115,            /* ambiguous_ref  */
  YYSYMBOL_attrdecllist = 116,             /* attrdecllist  */
  YYSYMBOL_attrdecl = 117,                 /* attrdecl  */
  YYSYMBOL_path = 118,                     /* path  */
  YYSYMBOL_datasection = 119,              /* datasection  */
  YYSYMBOL_datadecls = 120,                /* datadecls  */
  YYSYMBOL_datadecl = 121,                 /* datadecl  */
  YYSYMBOL_datalist = 122,                 /* datalist  */
  YYSYMBOL_datalist0 = 123,                /* datalist0  */
  YYSYMBOL_datalist1 = 124,                /* datalist1  */
  YYSYMBOL_dataitem = 125,                 /* dataitem  */
  YYSYMBOL_constdata = 126,                /* constdata  */
  YYSYMBOL_econstref = 127,                /* econstref  */
  YYSYMBOL_function = 128,                 /* function  */
  YYSYMBOL_arglist = 129,                  /* arglist  */
  YYSYMBOL_simpleconstant = 130,           /* simpleconstant  */
  YYSYMBOL_intlist = 131,                  /* intlist  */
  YYSYMBOL_constint = 132,                 /* constint  */
  YYSYMBOL_conststring = 133,              /* conststring  */
  YYSYMBOL_constbool = 134,                /* constbool  */
  YYSYMBOL_varident = 135,                 /* varident  */
  YYSYMBOL_ident = 136                     /* ident  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   420

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  159
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  276

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   314


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      65,    66,    67,     2,    63,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    68,    62,
       2,    64,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,     2,    61,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   243,   243,   249,   251,   258,   265,   265,   268,   277,
     267,   282,   283,   284,   288,   288,   290,   300,   300,   303,
     304,   305,   306,   309,   309,   312,   342,   344,   361,   370,
     382,   396,   429,   430,   433,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,   458,   461,   462,   463,
     466,   467,   470,   470,   472,   473,   477,   485,   495,   507,
     508,   509,   512,   513,   516,   516,   518,   540,   544,   548,
     577,   578,   581,   582,   586,   600,   604,   609,   638,   639,
     643,   644,   649,   659,   679,   690,   701,   720,   727,   727,
     730,   732,   734,   736,   738,   747,   758,   760,   762,   764,
     766,   768,   770,   772,   774,   776,   778,   780,   782,   784,
     786,   791,   798,   807,   808,   809,   812,   813,   816,   820,
     821,   825,   829,   830,   835,   836,   840,   841,   842,   843,
     844,   845,   849,   853,   857,   859,   864,   865,   866,   867,
     868,   869,   870,   871,   872,   873,   874,   875,   879,   880,
     884,   886,   888,   890,   895,   899,   900,   908,   909,   913
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "NC_UNLIMITED_K",
  "CHAR_K", "BYTE_K", "SHORT_K", "INT_K", "FLOAT_K", "DOUBLE_K", "UBYTE_K",
  "USHORT_K", "UINT_K", "INT64_K", "UINT64_K", "STRING_K", "IDENT",
  "TERMSTRING", "CHAR_CONST", "BYTE_CONST", "SHORT_CONST", "INT_CONST",
  "INT64_CONST", "UBYTE_CONST", "USHORT_CONST", "UINT_CONST",
  "UINT64_CONST", "FLOAT_CONST", "DOUBLE_CONST", "DIMENSIONS", "VARIABLES",
  "NETCDF", "DATA", "TYPES", "COMPOUND", "ENUM", "OPAQUE_", "OPAQUESTRING",
  "GROUP", "PATH", "FILLMARKER", "NIL", "_FILLVALUE", "_FORMAT",
  "_STORAGE", "_CHUNKSIZES", "_DEFLATELEVEL", "_SHUFFLE", "_ENDIANNESS",
  "_NOFILL", "_FLETCHER32", "_NCPROPS", "_ISNETCDF4", "_SUPERBLOCK",
  "_FILTER", "_CODECS", "_QUANTIZEBG", "_QUANTIZEGBR", "_QUANTIZEBR",
  "DATASETID", "'{'", "'}'", "';'", "','", "'='", "'('", "')'", "'*'",
  "':'", "$accept", "ncdesc", "datasetid", "rootgroup", "groupbody",
  "subgrouplist", "namedgroup", "$@1", "$@2", "typesection", "typedecls",
  "typename", "type_or_attr_decl", "typedecl", "optsemicolon", "enumdecl",
  "enumidlist", "enumid", "opaquedecl", "vlendecl", "compounddecl",
  "fields", "field", "primtype", "dimsection", "dimdecls",
  "dim_or_attr_decl", "dimdeclist", "dimdecl", "dimd", "vasection",
  "vadecls", "vadecl_or_attr", "vardecl", "varlist", "varspec", "dimspec",
  "dimlist", "dimref", "fieldlist", "fieldspec", "fielddimspec",
  "fielddimlist", "fielddim", "varref", "typeref", "ambiguous_ref",
  "attrdecllist", "attrdecl", "path", "datasection", "datadecls",
  "datadecl", "datalist", "datalist0", "datalist1", "dataitem",
  "constdata", "econstref", "function", "arglist", "simpleconstant",
  "intlist", "constint", "conststring", "constbool", "varident", "ident", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     123,   125,    59,    44,    61,    40,    41,    42,    58
};
#endif

#define YYPACT_NINF (-153)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-160)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -10,   -24,    30,  -153,   -18,  -153,   233,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,    20,  -153,  -153,   381,    -4,     5,   -16,  -153,
    -153,     4,    23,    45,    46,    50,   -28,    51,     3,   197,
      89,   233,    74,    74,    80,    82,   307,   106,  -153,  -153,
      -1,    63,    66,    67,    68,    69,    71,    72,    75,    77,
      78,    79,    81,    85,    86,   106,    87,   197,  -153,  -153,
      91,    91,    91,    91,   109,   246,    92,   233,   126,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,   307,  -153,    94,  -153,
    -153,  -153,  -153,  -153,  -153,  -153,    93,    99,    98,   100,
     307,    74,    82,    82,    80,    74,    80,    80,    74,    74,
      82,    82,    82,   307,   105,  -153,   146,  -153,  -153,  -153,
    -153,  -153,  -153,   106,   102,  -153,   233,   108,   110,  -153,
     107,  -153,   111,   233,   140,    27,   307,   256,  -153,   307,
     307,    94,  -153,   113,  -153,  -153,  -153,  -153,  -153,  -153,
    -153,  -153,  -153,  -153,  -153,    94,   381,   114,   117,   118,
     112,  -153,   106,    53,   233,   123,  -153,   345,  -153,   381,
    -153,  -153,  -153,   -43,  -153,   233,    94,    94,    82,   282,
     124,   106,  -153,   106,   106,   106,  -153,  -153,  -153,  -153,
    -153,   125,  -153,   120,  -153,   127,  -153,   128,   130,  -153,
     381,   129,   256,  -153,  -153,  -153,  -153,   134,  -153,   135,
    -153,   149,  -153,    62,  -153,   136,  -153,  -153,    -5,     1,
    -153,   307,   154,  -153,  -153,   151,  -153,   106,    -3,  -153,
    -153,   106,    82,  -153,  -153,   -34,  -153,  -153,    94,  -153,
     131,  -153,  -153,  -153,    14,  -153,  -153,  -153,     1,  -153,
     233,    -3,  -153,  -153,  -153,  -153
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     3,     0,     1,    88,     2,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
     159,   112,     0,     6,    87,     0,    85,    11,     0,    86,
     111,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      47,    88,     0,     0,     0,     0,   121,     0,     4,     7,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    17,
      23,    23,    23,    23,    87,     0,     0,    48,    59,    89,
     154,   110,    90,   150,   152,   151,   153,   156,   155,    91,
      92,   147,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   127,   128,   129,   121,   132,    93,   119,
     120,   122,   124,   130,   131,   126,   111,     0,     0,     0,
     121,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,    16,     0,    15,    24,    19,
      22,    21,    20,     0,     0,    18,    49,     0,    52,    54,
       0,    53,   111,    60,   113,     0,     0,     0,     8,   121,
     121,    96,    98,    99,   148,   101,   102,   103,   109,   100,
     104,   105,   106,   107,   108,    95,     0,     0,     0,     0,
       0,    50,     0,     0,    61,     0,    64,     0,    65,   114,
       5,   125,   123,     0,   134,    88,    97,    94,     0,     0,
       0,     0,    85,     0,     0,     0,    51,    55,    58,    57,
      56,     0,    62,   157,   158,    66,    67,    70,     0,    84,
     115,     0,     0,   133,     6,   149,    31,     0,    32,    34,
      75,    78,    29,     0,    26,     0,    30,    63,     0,     0,
      69,   121,     0,   116,   135,     9,    33,     0,     0,    77,
      25,     0,     0,   157,    68,     0,    72,    74,   118,   117,
       0,    76,    83,    82,     0,    80,    27,    28,     0,    71,
      88,     0,    79,    73,    10,    81
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -153,  -153,  -153,  -153,    22,    -6,  -153,  -153,  -153,  -153,
    -153,  -136,   153,  -153,    10,  -153,  -153,   -25,  -153,  -153,
    -153,  -153,    24,   -30,  -153,  -153,    76,  -153,    39,  -153,
    -153,  -153,    43,  -153,  -153,    25,  -153,  -153,    -2,  -153,
     -19,  -153,  -153,   -39,  -153,   -36,   -21,   -40,   -33,   -44,
    -153,  -153,    15,   -94,  -153,  -153,   115,  -153,  -153,  -153,
    -153,  -152,  -153,   -37,   -29,     2,  -153,   -22
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     4,     7,    23,    36,    49,   195,   260,    40,
      67,   134,    68,    69,   139,    70,   233,   234,    71,    72,
      73,   199,   200,    24,    78,   146,   147,   148,   149,   150,
     154,   184,   185,   186,   215,   216,   240,   255,   256,   229,
     230,   249,   264,   265,   218,    25,    26,    27,    28,    29,
     190,   220,   221,   108,   109,   110,   111,   112,   113,   114,
     193,   115,   163,    87,    88,    89,   217,    30
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      35,    79,   107,    75,    37,   194,    76,   178,    90,    74,
      47,   253,   155,    81,    82,    20,    64,    20,   262,    20,
     222,     1,   263,   223,   116,   117,   161,   214,   119,   268,
       5,    75,   269,    48,    76,     3,    20,    74,    39,   175,
      21,   118,     6,   135,   151,    51,    41,    52,    53,    54,
      55,    56,    57,    58,    37,   152,   209,    59,    60,    61,
      62,    63,   107,    31,    38,   196,   197,   232,    42,   236,
     244,    32,    33,    34,    83,    84,   107,   271,    85,    86,
     272,   140,   141,   142,   116,   164,   165,    43,   191,   107,
     156,    80,   162,   172,   173,   174,   167,    80,   116,   170,
     171,    83,    84,    83,    84,    85,    86,    85,    86,    44,
      45,   116,   107,   151,    46,   107,   107,   187,    77,    50,
     188,   135,    20,   250,   152,   251,   166,   120,   168,   169,
     121,   122,   123,   124,   116,   125,   126,   116,   116,   127,
     201,   128,   129,   130,   143,   131,   210,   258,   187,   132,
     133,   188,   136,   138,   145,   202,   153,   156,   157,   158,
     208,   225,   159,   201,   160,   176,    37,   177,   219,   179,
     181,   183,   189,   182,   206,   -58,   198,   204,   202,   231,
     203,   135,   235,   135,   205,   212,   228,   237,  -159,    47,
     238,   243,   270,   239,   241,   257,   246,   107,   247,   219,
     252,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,   248,   267,   259,   224,   245,   116,
     137,   207,   180,   227,   257,   231,   266,   211,   261,   235,
     274,    65,   275,    66,     0,   242,    21,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,   254,     0,    22,   273,     0,     0,     0,
       0,   192,    21,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,    21,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   144,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    21,     0,    20,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,     0,     0,     0,     0,
       0,     0,     0,   226,   103,     0,    21,   104,   105,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,   213,     0,     0,     0,     0,     0,   106,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
       0,     0,     0,     0,    21,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21
};

static const yytype_int16 yycheck[] =
{
      22,    41,    46,    39,    25,   157,    39,   143,    45,    39,
      38,    16,   106,    42,    43,    16,    38,    16,    21,    16,
      63,    31,    25,    66,    46,    47,   120,    32,    50,    63,
       0,    67,    66,    61,    67,    59,    16,    67,    33,   133,
      39,    42,    60,    65,    77,    42,    62,    44,    45,    46,
      47,    48,    49,    50,    75,    77,     3,    54,    55,    56,
      57,    58,   106,    43,    68,   159,   160,   203,    64,   205,
     222,    51,    52,    53,    21,    22,   120,    63,    25,    26,
      66,    71,    72,    73,   106,   122,   123,    64,    61,   133,
      63,    17,   121,   130,   131,   132,   125,    17,   120,   128,
     129,    21,    22,    21,    22,    25,    26,    25,    26,    64,
      64,   133,   156,   146,    64,   159,   160,   153,    29,    68,
     153,   143,    16,    61,   146,    63,   124,    64,   126,   127,
      64,    64,    64,    64,   156,    64,    64,   159,   160,    64,
     176,    64,    64,    64,    35,    64,   183,   241,   184,    64,
      64,   184,    65,    62,    62,   176,    30,    63,    65,    60,
     182,   198,    64,   199,    64,    60,   187,    21,   189,    67,
      62,    64,    32,    63,    62,    64,    63,    60,   199,   201,
      66,   203,   204,   205,    66,    62,    62,    62,    68,    38,
      63,    62,    61,    65,    64,   239,    62,   241,    63,   220,
      64,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    65,   252,    62,   195,   224,   241,
      67,   182,   146,   199,   268,   247,   251,   184,   247,   251,
     270,    34,   271,    36,    -1,   220,    39,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,   238,    -1,    68,   268,    -1,    -1,    -1,
      -1,   156,    39,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    39,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    37,    -1,    39,    40,    41,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    -1,    -1,    -1,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    39,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    31,    70,    59,    71,     0,    60,    72,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    39,    68,    73,    92,   114,   115,   116,   117,   118,
     136,    43,    51,    52,    53,   136,    74,   115,    68,    33,
      78,    62,    64,    64,    64,    64,    64,    38,    61,    75,
      68,    42,    44,    45,    46,    47,    48,    49,    50,    54,
      55,    56,    57,    58,   136,    34,    36,    79,    81,    82,
      84,    87,    88,    89,    92,   114,   117,    29,    93,   116,
      17,   133,   133,    21,    22,    25,    26,   132,   133,   134,
     132,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    37,    40,    41,    60,   118,   122,   123,
     124,   125,   126,   127,   128,   130,   136,   136,    42,   136,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    64,    64,    80,   136,    65,    81,    62,    83,
      83,    83,    83,    35,    65,    62,    94,    95,    96,    97,
      98,   117,   136,    30,    99,   122,    63,    65,    60,    64,
      64,   122,   133,   131,   132,   132,   134,   133,   134,   134,
     133,   133,   132,   132,   132,   122,    60,    21,    80,    67,
      95,    62,    63,    64,   100,   101,   102,   114,   117,    32,
     119,    61,   125,   129,   130,    76,   122,   122,    63,    90,
      91,   114,   115,    66,    60,    66,    62,    97,   136,     3,
     132,   101,    62,    16,    32,   103,   104,   135,   113,   115,
     120,   121,    63,    66,    73,   132,    61,    91,    62,   108,
     109,   136,    80,    85,    86,   136,    80,    62,    63,    65,
     105,    64,   121,    62,   130,    74,    62,    63,    65,   110,
      61,    63,    64,    16,   104,   106,   107,   118,   122,    62,
      77,   109,    21,    25,   111,   112,    86,   132,    63,    66,
      61,    63,    66,   107,   116,   112
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    69,    70,    71,    72,    73,    74,    74,    76,    77,
      75,    78,    78,    78,    79,    79,    80,    81,    81,    82,
      82,    82,    82,    83,    83,    84,    85,    85,    86,    87,
      88,    89,    90,    90,    91,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    93,    93,    93,
      94,    94,    95,    95,    96,    96,    97,    97,    98,    99,
      99,    99,   100,   100,   101,   101,   102,   103,   103,   104,
     105,   105,   106,   106,   107,   108,   108,   109,   110,   110,
     111,   111,   112,   112,   113,   114,   115,   115,   116,   116,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   118,   118,   119,   119,   119,   120,   120,   121,   122,
     122,   123,   124,   124,   125,   125,   126,   126,   126,   126,
     126,   126,   127,   128,   129,   129,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   131,   131,
     132,   132,   132,   132,   133,   134,   134,   135,   135,   136
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     4,     5,     0,     2,     0,     0,
       9,     0,     1,     2,     1,     2,     1,     1,     2,     2,
       2,     2,     2,     0,     1,     6,     1,     3,     3,     5,
       5,     5,     2,     3,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     2,
       2,     3,     1,     1,     1,     3,     3,     3,     1,     0,
       1,     2,     2,     3,     1,     1,     2,     1,     3,     2,
       0,     3,     1,     3,     1,     1,     3,     2,     0,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     0,     3,
       4,     4,     4,     4,     6,     5,     5,     6,     5,     5,
       5,     5,     5,     5,     5,     5,     5,     5,     5,     5,
       4,     1,     1,     0,     1,     2,     2,     3,     3,     1,
       1,     0,     1,     3,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* ncdesc: NETCDF datasetid rootgroup  */
#line 246 "ncgen.y"
        {if (error_count > 0) YYABORT;}
#line 1864 "ncgeny.c"
    break;

  case 3: /* datasetid: DATASETID  */
#line 249 "ncgen.y"
                     {createrootgroup(datasetname);}
#line 1870 "ncgeny.c"
    break;

  case 8: /* $@1: %empty  */
#line 268 "ncgen.y"
            {
		Symbol* id = (yyvsp[-1].sym);
                markcdf4("Group specification");
		if(creategroup(id) == NULL)
                    yyerror("duplicate group declaration within parent group for %s",
                                id->name);
            }
#line 1882 "ncgeny.c"
    break;

  case 9: /* $@2: %empty  */
#line 277 "ncgen.y"
            {listpop(groupstack);}
#line 1888 "ncgeny.c"
    break;

  case 12: /* typesection: TYPES  */
#line 283 "ncgen.y"
                        {}
#line 1894 "ncgeny.c"
    break;

  case 13: /* typesection: TYPES typedecls  */
#line 285 "ncgen.y"
                        {markcdf4("Type specification");}
#line 1900 "ncgeny.c"
    break;

  case 16: /* typename: ident  */
#line 291 "ncgen.y"
            { /* Use when defining a type */
              (yyvsp[0].sym)->objectclass = NC_TYPE;
              if(dupobjectcheck(NC_TYPE,(yyvsp[0].sym)))
                    yyerror("duplicate type declaration for %s",
                            (yyvsp[0].sym)->name);
              listpush(typdefs,(void*)(yyvsp[0].sym));
	    }
#line 1912 "ncgeny.c"
    break;

  case 17: /* type_or_attr_decl: typedecl  */
#line 300 "ncgen.y"
                            {}
#line 1918 "ncgeny.c"
    break;

  case 18: /* type_or_attr_decl: attrdecl ';'  */
#line 300 "ncgen.y"
                                              {}
#line 1924 "ncgeny.c"
    break;

  case 25: /* enumdecl: primtype ENUM typename '{' enumidlist '}'  */
#line 314 "ncgen.y"
              {
		int i;
                addtogroup((yyvsp[-3].sym)); /* sets prefix*/
                (yyvsp[-3].sym)->objectclass=NC_TYPE;
                (yyvsp[-3].sym)->subclass=NC_ENUM;
                (yyvsp[-3].sym)->typ.basetype=(yyvsp[-5].sym);
                (yyvsp[-3].sym)->typ.size = (yyvsp[-5].sym)->typ.size;
                (yyvsp[-3].sym)->typ.alignment = (yyvsp[-5].sym)->typ.alignment;
                stackbase=(yyvsp[-1].mark);
                stacklen=listlength(stack);
                (yyvsp[-3].sym)->subnodes = listnew();
                /* Variety of field fixups*/
		/* 1. add in the enum values*/
		/* 2. make this type be their container*/
		/* 3. make constant names visible in the group*/
		/* 4. set field basetype to be same as enum basetype*/
                for(i=stackbase;i<stacklen;i++) {
                   Symbol* eid = (Symbol*)listget(stack,i);
		   assert(eid->subclass == NC_ECONST);
		   addtogroup(eid);
                   listpush((yyvsp[-3].sym)->subnodes,(void*)eid);
                   eid->container = (yyvsp[-3].sym);
		   eid->typ.basetype = (yyvsp[-3].sym)->typ.basetype;
                }
                listsetlength(stack,stackbase);/* remove stack nodes*/
              }
#line 1955 "ncgeny.c"
    break;

  case 26: /* enumidlist: enumid  */
#line 343 "ncgen.y"
                {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 1961 "ncgeny.c"
    break;

  case 27: /* enumidlist: enumidlist ',' enumid  */
#line 345 "ncgen.y"
                {
		    int i;
		    (yyval.mark)=(yyvsp[-2].mark);
		    /* check for duplicates*/
		    stackbase=(yyvsp[-2].mark);
		    stacklen=listlength(stack);
		    for(i=stackbase;i<stacklen;i++) {
		      Symbol* elem = (Symbol*)listget(stack,i);
		      if(strcmp((yyvsp[0].sym)->name,elem->name)==0)
  	                yyerror("duplicate enum declaration for %s",
        	                 elem->name);
		    }
		    listpush(stack,(void*)(yyvsp[0].sym));
		}
#line 1980 "ncgeny.c"
    break;

  case 28: /* enumid: ident '=' constint  */
#line 362 "ncgen.y"
        {
            (yyvsp[-2].sym)->objectclass=NC_TYPE;
            (yyvsp[-2].sym)->subclass=NC_ECONST;
            (yyvsp[-2].sym)->typ.econst=(yyvsp[0].constant);
	    (yyval.sym)=(yyvsp[-2].sym);
        }
#line 1991 "ncgeny.c"
    break;

  case 29: /* opaquedecl: OPAQUE_ '(' INT_CONST ')' typename  */
#line 371 "ncgen.y"
                {
		    vercheck(NC_OPAQUE);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.typecode=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.size=int32_val;
                    (void)ncaux_class_alignment(NC_OPAQUE,&(yyvsp[0].sym)->typ.alignment);
                }
#line 2005 "ncgeny.c"
    break;

  case 30: /* vlendecl: typeref '(' '*' ')' typename  */
#line 383 "ncgen.y"
                {
                    Symbol* basetype = (yyvsp[-4].sym);
		    vercheck(NC_VLEN);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_VLEN;
                    (yyvsp[0].sym)->typ.basetype=basetype;
                    (yyvsp[0].sym)->typ.typecode=NC_VLEN;
                    (yyvsp[0].sym)->typ.size=VLENSIZE;
                    (void)ncaux_class_alignment(NC_VLEN,&(yyvsp[0].sym)->typ.alignment);
                }
#line 2021 "ncgeny.c"
    break;

  case 31: /* compounddecl: COMPOUND typename '{' fields '}'  */
#line 397 "ncgen.y"
          {
	    int i,j;
	    vercheck(NC_COMPOUND);
            addtogroup((yyvsp[-3].sym));
	    /* check for duplicate field names*/
	    stackbase=(yyvsp[-1].mark);
	    stacklen=listlength(stack);
	    for(i=stackbase;i<stacklen;i++) {
	      Symbol* elem1 = (Symbol*)listget(stack,i);
	      for(j=i+1;j<stacklen;j++) {
	          Symbol* elem2 = (Symbol*)listget(stack,j);
	          if(strcmp(elem1->name,elem2->name)==0) {
	            yyerror("duplicate field declaration for %s",elem1->name);
		  }
	      }
	    }
	    (yyvsp[-3].sym)->objectclass=NC_TYPE;
            (yyvsp[-3].sym)->subclass=NC_COMPOUND;
            (yyvsp[-3].sym)->typ.basetype=NULL;
            (yyvsp[-3].sym)->typ.typecode=NC_COMPOUND;
	    (yyvsp[-3].sym)->subnodes = listnew();
	    /* Add in the fields*/
	    for(i=stackbase;i<stacklen;i++) {
	        Symbol* fsym = (Symbol*)listget(stack,i);
		fsym->container = (yyvsp[-3].sym);
 	        listpush((yyvsp[-3].sym)->subnodes,(void*)fsym);
	    }
	    listsetlength(stack,stackbase);/* remove stack nodes*/
          }
#line 2055 "ncgeny.c"
    break;

  case 32: /* fields: field ';'  */
#line 429 "ncgen.y"
                    {(yyval.mark)=(yyvsp[-1].mark);}
#line 2061 "ncgeny.c"
    break;

  case 33: /* fields: fields field ';'  */
#line 430 "ncgen.y"
                              {(yyval.mark)=(yyvsp[-2].mark);}
#line 2067 "ncgeny.c"
    break;

  case 34: /* field: typeref fieldlist  */
#line 434 "ncgen.y"
        {
	    int i;
	    (yyval.mark)=(yyvsp[0].mark);
	    stackbase=(yyvsp[0].mark);
	    stacklen=listlength(stack);
	    /* process each field in the fieldlist*/
            for(i=stackbase;i<stacklen;i++) {
                Symbol* f = (Symbol*)listget(stack,i);
		f->typ.basetype = (yyvsp[-1].sym);
            }
        }
#line 2083 "ncgeny.c"
    break;

  case 35: /* primtype: CHAR_K  */
#line 447 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_CHAR]; }
#line 2089 "ncgeny.c"
    break;

  case 36: /* primtype: BYTE_K  */
#line 448 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_BYTE]; }
#line 2095 "ncgeny.c"
    break;

  case 37: /* primtype: SHORT_K  */
#line 449 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_SHORT]; }
#line 2101 "ncgeny.c"
    break;

  case 38: /* primtype: INT_K  */
#line 450 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_INT]; }
#line 2107 "ncgeny.c"
    break;

  case 39: /* primtype: FLOAT_K  */
#line 451 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_FLOAT]; }
#line 2113 "ncgeny.c"
    break;

  case 40: /* primtype: DOUBLE_K  */
#line 452 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_DOUBLE]; }
#line 2119 "ncgeny.c"
    break;

  case 41: /* primtype: UBYTE_K  */
#line 453 "ncgen.y"
                           { vercheck(NC_UBYTE); (yyval.sym) = primsymbols[NC_UBYTE]; }
#line 2125 "ncgeny.c"
    break;

  case 42: /* primtype: USHORT_K  */
#line 454 "ncgen.y"
                           { vercheck(NC_USHORT); (yyval.sym) = primsymbols[NC_USHORT]; }
#line 2131 "ncgeny.c"
    break;

  case 43: /* primtype: UINT_K  */
#line 455 "ncgen.y"
                           { vercheck(NC_UINT); (yyval.sym) = primsymbols[NC_UINT]; }
#line 2137 "ncgeny.c"
    break;

  case 44: /* primtype: INT64_K  */
#line 456 "ncgen.y"
                            { vercheck(NC_INT64); (yyval.sym) = primsymbols[NC_INT64]; }
#line 2143 "ncgeny.c"
    break;

  case 45: /* primtype: UINT64_K  */
#line 457 "ncgen.y"
                             { vercheck(NC_UINT64); (yyval.sym) = primsymbols[NC_UINT64]; }
#line 2149 "ncgeny.c"
    break;

  case 46: /* primtype: STRING_K  */
#line 458 "ncgen.y"
                             { vercheck(NC_STRING); (yyval.sym) = primsymbols[NC_STRING]; }
#line 2155 "ncgeny.c"
    break;

  case 48: /* dimsection: DIMENSIONS  */
#line 462 "ncgen.y"
                             {}
#line 2161 "ncgeny.c"
    break;

  case 49: /* dimsection: DIMENSIONS dimdecls  */
#line 463 "ncgen.y"
                                      {}
#line 2167 "ncgeny.c"
    break;

  case 52: /* dim_or_attr_decl: dimdeclist  */
#line 470 "ncgen.y"
                             {}
#line 2173 "ncgeny.c"
    break;

  case 53: /* dim_or_attr_decl: attrdecl  */
#line 470 "ncgen.y"
                                           {}
#line 2179 "ncgeny.c"
    break;

  case 56: /* dimdecl: dimd '=' constint  */
#line 478 "ncgen.y"
              {
		(yyvsp[-2].sym)->dim.declsize = (size_t)extractint((yyvsp[0].constant));
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = %llu\n",(yyvsp[-2].sym)->name,(unsigned long long)(yyvsp[-2].sym)->dim.declsize);
#endif
		reclaimconstant((yyvsp[0].constant));
	      }
#line 2191 "ncgeny.c"
    break;

  case 57: /* dimdecl: dimd '=' NC_UNLIMITED_K  */
#line 486 "ncgen.y"
                   {
		        (yyvsp[-2].sym)->dim.declsize = NC_UNLIMITED;
		        (yyvsp[-2].sym)->dim.isunlimited = 1;
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = UNLIMITED\n",(yyvsp[-2].sym)->name);
#endif
		   }
#line 2203 "ncgeny.c"
    break;

  case 58: /* dimd: ident  */
#line 496 "ncgen.y"
                   {
                     (yyvsp[0].sym)->objectclass=NC_DIM;
                     if(dupobjectcheck(NC_DIM,(yyvsp[0].sym)))
                        yyerror( "Duplicate dimension declaration for %s",
                                (yyvsp[0].sym)->name);
		     addtogroup((yyvsp[0].sym));
		     (yyval.sym)=(yyvsp[0].sym);
		     listpush(dimdefs,(void*)(yyvsp[0].sym));
                   }
#line 2217 "ncgeny.c"
    break;

  case 60: /* vasection: VARIABLES  */
#line 508 "ncgen.y"
                            {}
#line 2223 "ncgeny.c"
    break;

  case 61: /* vasection: VARIABLES vadecls  */
#line 509 "ncgen.y"
                                    {}
#line 2229 "ncgeny.c"
    break;

  case 64: /* vadecl_or_attr: vardecl  */
#line 516 "ncgen.y"
                        {}
#line 2235 "ncgeny.c"
    break;

  case 65: /* vadecl_or_attr: attrdecl  */
#line 516 "ncgen.y"
                                      {}
#line 2241 "ncgeny.c"
    break;

  case 66: /* vardecl: typeref varlist  */
#line 519 "ncgen.y"
                {
		    int i;
		    stackbase=(yyvsp[0].mark);
		    stacklen=listlength(stack);
		    /* process each variable in the varlist*/
	            for(i=stackbase;i<stacklen;i++) {
	                Symbol* sym = (Symbol*)listget(stack,i);
			sym->objectclass = NC_VAR;
		        if(dupobjectcheck(NC_VAR,sym)) {
                            yyerror("Duplicate variable declaration for %s",
                                    sym->name);
			} else {
		  	    sym->typ.basetype = (yyvsp[-1].sym);
	                    addtogroup(sym);
		            listpush(vardefs,(void*)sym);
			}
		    }
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		}
#line 2265 "ncgeny.c"
    break;

  case 67: /* varlist: varspec  */
#line 541 "ncgen.y"
                {(yyval.mark)=listlength(stack);
                 listpush(stack,(void*)(yyvsp[0].sym));
		}
#line 2273 "ncgeny.c"
    break;

  case 68: /* varlist: varlist ',' varspec  */
#line 545 "ncgen.y"
                {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2279 "ncgeny.c"
    break;

  case 69: /* varspec: varident dimspec  */
#line 549 "ncgen.y"
                    {
		    int i;
		    Dimset dimset;
		    Symbol* var = (yyvsp[-1].sym); /* for debugging */
		    stacklen=listlength(stack);
		    stackbase=(yyvsp[0].mark);
		    count = stacklen - stackbase;
		    if(count >= NC_MAX_VAR_DIMS) {
			yyerror("%s has too many dimensions",(yyvsp[-1].sym)->name);
			count = NC_MAX_VAR_DIMS - 1;
			stacklen = stackbase + count;
		    }
  	            dimset.ndims = count;
		    /* extract the actual dimensions*/
		    if(dimset.ndims > 0) {
		        for(i=0;i<count;i++) {
			    Symbol* dsym = (Symbol*)listget(stack,stackbase+i);
			    dimset.dimsyms[i] = dsym;
			}
			var->typ.dimset = dimset;
		    }
		    var->typ.basetype = NULL; /* not yet known*/
                    var->objectclass=NC_VAR;
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		    (yyval.sym) = var;
		    }
#line 2310 "ncgeny.c"
    break;

  case 70: /* dimspec: %empty  */
#line 577 "ncgen.y"
                            {(yyval.mark)=listlength(stack);}
#line 2316 "ncgeny.c"
    break;

  case 71: /* dimspec: '(' dimlist ')'  */
#line 578 "ncgen.y"
                                  {(yyval.mark)=(yyvsp[-1].mark);}
#line 2322 "ncgeny.c"
    break;

  case 72: /* dimlist: dimref  */
#line 581 "ncgen.y"
                       {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2328 "ncgeny.c"
    break;

  case 73: /* dimlist: dimlist ',' dimref  */
#line 583 "ncgen.y"
                    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2334 "ncgeny.c"
    break;

  case 74: /* dimref: path  */
#line 587 "ncgen.y"
            {Symbol* dimsym = (yyvsp[0].sym);
		dimsym->objectclass = NC_DIM;
		/* Find the actual dimension*/
		dimsym = locate(dimsym);
		if(dimsym == NULL) {
		    derror("Undefined or forward referenced dimension: %s",(yyvsp[0].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=dimsym;
	    }
#line 2349 "ncgeny.c"
    break;

  case 75: /* fieldlist: fieldspec  */
#line 601 "ncgen.y"
            {(yyval.mark)=listlength(stack);
             listpush(stack,(void*)(yyvsp[0].sym));
	    }
#line 2357 "ncgeny.c"
    break;

  case 76: /* fieldlist: fieldlist ',' fieldspec  */
#line 605 "ncgen.y"
            {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2363 "ncgeny.c"
    break;

  case 77: /* fieldspec: ident fielddimspec  */
#line 610 "ncgen.y"
            {
		int i;
		Dimset dimset;
		stackbase=(yyvsp[0].mark);
		stacklen=listlength(stack);
		count = stacklen - stackbase;
		if(count >= NC_MAX_VAR_DIMS) {
		    yyerror("%s has too many dimensions",(yyvsp[-1].sym)->name);
		    count = NC_MAX_VAR_DIMS - 1;
		    stacklen = stackbase + count;
		}
  	        dimset.ndims = count;
		if(count > 0) {
		    /* extract the actual dimensions*/
		    for(i=0;i<count;i++) {
		        Symbol* dsym = (Symbol*)listget(stack,stackbase+i);
		        dimset.dimsyms[i] = dsym;
		    }
		    (yyvsp[-1].sym)->typ.dimset = dimset;
		}
		(yyvsp[-1].sym)->typ.basetype = NULL; /* not yet known*/
                (yyvsp[-1].sym)->objectclass=NC_TYPE;
                (yyvsp[-1].sym)->subclass=NC_FIELD;
		listsetlength(stack,stackbase);/* remove stack nodes*/
		(yyval.sym) = (yyvsp[-1].sym);
	    }
#line 2394 "ncgeny.c"
    break;

  case 78: /* fielddimspec: %empty  */
#line 638 "ncgen.y"
                                 {(yyval.mark)=listlength(stack);}
#line 2400 "ncgeny.c"
    break;

  case 79: /* fielddimspec: '(' fielddimlist ')'  */
#line 639 "ncgen.y"
                                       {(yyval.mark)=(yyvsp[-1].mark);}
#line 2406 "ncgeny.c"
    break;

  case 80: /* fielddimlist: fielddim  */
#line 643 "ncgen.y"
                   {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2412 "ncgeny.c"
    break;

  case 81: /* fielddimlist: fielddimlist ',' fielddim  */
#line 645 "ncgen.y"
            {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2418 "ncgeny.c"
    break;

  case 82: /* fielddim: UINT_CONST  */
#line 650 "ncgen.y"
            {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     sprintf(anon,"const%u",uint32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = uint32_val;
	    }
#line 2432 "ncgeny.c"
    break;

  case 83: /* fielddim: INT_CONST  */
#line 660 "ncgen.y"
            {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     if(int32_val <= 0) {
		derror("field dimension must be positive");
		YYABORT;
	     }
	     sprintf(anon,"const%d",int32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = int32_val;
	    }
#line 2450 "ncgeny.c"
    break;

  case 84: /* varref: ambiguous_ref  */
#line 680 "ncgen.y"
            {Symbol* vsym = (yyvsp[0].sym);
		if(vsym->objectclass != NC_VAR) {
		    derror("Undefined or forward referenced variable: %s",vsym->name);
		    YYABORT;
		}
		(yyval.sym)=vsym;
	    }
#line 2462 "ncgeny.c"
    break;

  case 85: /* typeref: ambiguous_ref  */
#line 691 "ncgen.y"
            {Symbol* tsym = (yyvsp[0].sym);
		if(tsym->objectclass != NC_TYPE) {
		    derror("Undefined or forward referenced type: %s",tsym->name);
		    YYABORT;
		}
		(yyval.sym)=tsym;
	    }
#line 2474 "ncgeny.c"
    break;

  case 86: /* ambiguous_ref: path  */
#line 702 "ncgen.y"
            {Symbol* tvsym = (yyvsp[0].sym); Symbol* sym;
		/* disambiguate*/
		tvsym->objectclass = NC_VAR;
		sym = locate(tvsym);
		if(sym == NULL) {
		    tvsym->objectclass = NC_TYPE;
		    sym = locate(tvsym);
		    if(tvsym == NULL) {
		        derror("Undefined or forward referenced name: %s",(yyvsp[0].sym)->name);
		        YYABORT;
		    } else tvsym = sym;
		} else tvsym = sym;
		if(tvsym == NULL) {
		    derror("Undefined name (line %d): %s",(yyvsp[0].sym)->lineno,(yyvsp[0].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=tvsym;
	    }
#line 2497 "ncgeny.c"
    break;

  case 87: /* ambiguous_ref: primtype  */
#line 720 "ncgen.y"
                   {(yyval.sym)=(yyvsp[0].sym);}
#line 2503 "ncgeny.c"
    break;

  case 88: /* attrdecllist: %empty  */
#line 727 "ncgen.y"
                        {}
#line 2509 "ncgeny.c"
    break;

  case 89: /* attrdecllist: attrdecl ';' attrdecllist  */
#line 727 "ncgen.y"
                                                       {}
#line 2515 "ncgeny.c"
    break;

  case 90: /* attrdecl: ':' _NCPROPS '=' conststring  */
#line 731 "ncgen.y"
            {(yyval.sym) = makespecial(_NCPROPS_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2521 "ncgeny.c"
    break;

  case 91: /* attrdecl: ':' _ISNETCDF4 '=' constbool  */
#line 733 "ncgen.y"
            {(yyval.sym) = makespecial(_ISNETCDF4_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2527 "ncgeny.c"
    break;

  case 92: /* attrdecl: ':' _SUPERBLOCK '=' constint  */
#line 735 "ncgen.y"
            {(yyval.sym) = makespecial(_SUPERBLOCK_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2533 "ncgeny.c"
    break;

  case 93: /* attrdecl: ':' ident '=' datalist  */
#line 737 "ncgen.y"
            { (yyval.sym)=makeattribute((yyvsp[-2].sym),NULL,NULL,(yyvsp[0].datalist),ATTRGLOBAL);}
#line 2539 "ncgeny.c"
    break;

  case 94: /* attrdecl: typeref ambiguous_ref ':' ident '=' datalist  */
#line 739 "ncgen.y"
            {Symbol* tsym = (yyvsp[-5].sym); Symbol* vsym = (yyvsp[-4].sym); Symbol* asym = (yyvsp[-2].sym);
		if(vsym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,vsym,tsym,(yyvsp[0].datalist),ATTRVAR);
		} else {
		    derror("Doubly typed attribute: %s",asym->name);
		    YYABORT;
		}
	    }
#line 2552 "ncgeny.c"
    break;

  case 95: /* attrdecl: ambiguous_ref ':' ident '=' datalist  */
#line 748 "ncgen.y"
            {Symbol* sym = (yyvsp[-4].sym); Symbol* asym = (yyvsp[-2].sym);
		if(sym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,sym,NULL,(yyvsp[0].datalist),ATTRVAR);
		} else if(sym->objectclass == NC_TYPE) {
		    (yyval.sym)=makeattribute(asym,NULL,sym,(yyvsp[0].datalist),ATTRGLOBAL);
		} else {
		    derror("Attribute prefix not a variable or type: %s",asym->name);
		    YYABORT;
		}
	    }
#line 2567 "ncgeny.c"
    break;

  case 96: /* attrdecl: ambiguous_ref ':' _FILLVALUE '=' datalist  */
#line 759 "ncgen.y"
            {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),ISLIST);}
#line 2573 "ncgeny.c"
    break;

  case 97: /* attrdecl: typeref ambiguous_ref ':' _FILLVALUE '=' datalist  */
#line 761 "ncgen.y"
            {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),(yyvsp[-5].sym),(void*)(yyvsp[0].datalist),ISLIST);}
#line 2579 "ncgeny.c"
    break;

  case 98: /* attrdecl: ambiguous_ref ':' _STORAGE '=' conststring  */
#line 763 "ncgen.y"
            {(yyval.sym) = makespecial(_STORAGE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2585 "ncgeny.c"
    break;

  case 99: /* attrdecl: ambiguous_ref ':' _CHUNKSIZES '=' intlist  */
#line 765 "ncgen.y"
            {(yyval.sym) = makespecial(_CHUNKSIZES_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),ISLIST);}
#line 2591 "ncgeny.c"
    break;

  case 100: /* attrdecl: ambiguous_ref ':' _FLETCHER32 '=' constbool  */
#line 767 "ncgen.y"
            {(yyval.sym) = makespecial(_FLETCHER32_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2597 "ncgeny.c"
    break;

  case 101: /* attrdecl: ambiguous_ref ':' _DEFLATELEVEL '=' constint  */
#line 769 "ncgen.y"
            {(yyval.sym) = makespecial(_DEFLATE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2603 "ncgeny.c"
    break;

  case 102: /* attrdecl: ambiguous_ref ':' _SHUFFLE '=' constbool  */
#line 771 "ncgen.y"
            {(yyval.sym) = makespecial(_SHUFFLE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2609 "ncgeny.c"
    break;

  case 103: /* attrdecl: ambiguous_ref ':' _ENDIANNESS '=' conststring  */
#line 773 "ncgen.y"
            {(yyval.sym) = makespecial(_ENDIAN_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2615 "ncgeny.c"
    break;

  case 104: /* attrdecl: ambiguous_ref ':' _FILTER '=' conststring  */
#line 775 "ncgen.y"
            {(yyval.sym) = makespecial(_FILTER_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2621 "ncgeny.c"
    break;

  case 105: /* attrdecl: ambiguous_ref ':' _CODECS '=' conststring  */
#line 777 "ncgen.y"
            {(yyval.sym) = makespecial(_CODECS_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2627 "ncgeny.c"
    break;

  case 106: /* attrdecl: ambiguous_ref ':' _QUANTIZEBG '=' constint  */
#line 779 "ncgen.y"
            {(yyval.sym) = makespecial(_QUANTIZEBG_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2633 "ncgeny.c"
    break;

  case 107: /* attrdecl: ambiguous_ref ':' _QUANTIZEGBR '=' constint  */
#line 781 "ncgen.y"
            {(yyval.sym) = makespecial(_QUANTIZEGBR_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2639 "ncgeny.c"
    break;

  case 108: /* attrdecl: ambiguous_ref ':' _QUANTIZEBR '=' constint  */
#line 783 "ncgen.y"
            {(yyval.sym) = makespecial(_QUANTIZEBR_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2645 "ncgeny.c"
    break;

  case 109: /* attrdecl: ambiguous_ref ':' _NOFILL '=' constbool  */
#line 785 "ncgen.y"
            {(yyval.sym) = makespecial(_NOFILL_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2651 "ncgeny.c"
    break;

  case 110: /* attrdecl: ':' _FORMAT '=' conststring  */
#line 787 "ncgen.y"
            {(yyval.sym) = makespecial(_FORMAT_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2657 "ncgeny.c"
    break;

  case 111: /* path: ident  */
#line 792 "ncgen.y"
            {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=0;
                setpathcurrent((yyvsp[0].sym));
	    }
#line 2668 "ncgeny.c"
    break;

  case 112: /* path: PATH  */
#line 799 "ncgen.y"
            {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=1;
	        /* path is set in ncgen.l*/
	    }
#line 2679 "ncgeny.c"
    break;

  case 114: /* datasection: DATA  */
#line 808 "ncgen.y"
                       {}
#line 2685 "ncgeny.c"
    break;

  case 115: /* datasection: DATA datadecls  */
#line 809 "ncgen.y"
                                 {}
#line 2691 "ncgeny.c"
    break;

  case 118: /* datadecl: varref '=' datalist  */
#line 817 "ncgen.y"
                   {(yyvsp[-2].sym)->data = (yyvsp[0].datalist);}
#line 2697 "ncgeny.c"
    break;

  case 119: /* datalist: datalist0  */
#line 820 "ncgen.y"
                    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2703 "ncgeny.c"
    break;

  case 120: /* datalist: datalist1  */
#line 821 "ncgen.y"
                    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2709 "ncgeny.c"
    break;

  case 121: /* datalist0: %empty  */
#line 825 "ncgen.y"
                  {(yyval.datalist) = builddatalist(0);}
#line 2715 "ncgeny.c"
    break;

  case 122: /* datalist1: dataitem  */
#line 829 "ncgen.y"
                   {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2721 "ncgeny.c"
    break;

  case 123: /* datalist1: datalist ',' dataitem  */
#line 831 "ncgen.y"
            {dlappend((yyvsp[-2].datalist),((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist); }
#line 2727 "ncgeny.c"
    break;

  case 124: /* dataitem: constdata  */
#line 835 "ncgen.y"
                    {(yyval.constant)=(yyvsp[0].constant);}
#line 2733 "ncgeny.c"
    break;

  case 125: /* dataitem: '{' datalist '}'  */
#line 836 "ncgen.y"
                           {(yyval.constant)=builddatasublist((yyvsp[-1].datalist));}
#line 2739 "ncgeny.c"
    break;

  case 126: /* constdata: simpleconstant  */
#line 840 "ncgen.y"
                              {(yyval.constant)=(yyvsp[0].constant);}
#line 2745 "ncgeny.c"
    break;

  case 127: /* constdata: OPAQUESTRING  */
#line 841 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_OPAQUE);}
#line 2751 "ncgeny.c"
    break;

  case 128: /* constdata: FILLMARKER  */
#line 842 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_FILLVALUE);}
#line 2757 "ncgeny.c"
    break;

  case 129: /* constdata: NIL  */
#line 843 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_NIL);}
#line 2763 "ncgeny.c"
    break;

  case 130: /* constdata: econstref  */
#line 844 "ncgen.y"
                        {(yyval.constant)=(yyvsp[0].constant);}
#line 2769 "ncgeny.c"
    break;

  case 132: /* econstref: path  */
#line 849 "ncgen.y"
             {(yyval.constant) = makeenumconstref((yyvsp[0].sym));}
#line 2775 "ncgeny.c"
    break;

  case 133: /* function: ident '(' arglist ')'  */
#line 853 "ncgen.y"
                              {(yyval.constant)=evaluate((yyvsp[-3].sym),(yyvsp[-1].datalist));}
#line 2781 "ncgeny.c"
    break;

  case 134: /* arglist: simpleconstant  */
#line 858 "ncgen.y"
            {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2787 "ncgeny.c"
    break;

  case 135: /* arglist: arglist ',' simpleconstant  */
#line 860 "ncgen.y"
            {dlappend((yyvsp[-2].datalist),((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist);}
#line 2793 "ncgeny.c"
    break;

  case 136: /* simpleconstant: CHAR_CONST  */
#line 864 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_CHAR);}
#line 2799 "ncgeny.c"
    break;

  case 137: /* simpleconstant: BYTE_CONST  */
#line 865 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_BYTE);}
#line 2805 "ncgeny.c"
    break;

  case 138: /* simpleconstant: SHORT_CONST  */
#line 866 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_SHORT);}
#line 2811 "ncgeny.c"
    break;

  case 139: /* simpleconstant: INT_CONST  */
#line 867 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_INT);}
#line 2817 "ncgeny.c"
    break;

  case 140: /* simpleconstant: INT64_CONST  */
#line 868 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2823 "ncgeny.c"
    break;

  case 141: /* simpleconstant: UBYTE_CONST  */
#line 869 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UBYTE);}
#line 2829 "ncgeny.c"
    break;

  case 142: /* simpleconstant: USHORT_CONST  */
#line 870 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_USHORT);}
#line 2835 "ncgeny.c"
    break;

  case 143: /* simpleconstant: UINT_CONST  */
#line 871 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2841 "ncgeny.c"
    break;

  case 144: /* simpleconstant: UINT64_CONST  */
#line 872 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2847 "ncgeny.c"
    break;

  case 145: /* simpleconstant: FLOAT_CONST  */
#line 873 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_FLOAT);}
#line 2853 "ncgeny.c"
    break;

  case 146: /* simpleconstant: DOUBLE_CONST  */
#line 874 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_DOUBLE);}
#line 2859 "ncgeny.c"
    break;

  case 147: /* simpleconstant: TERMSTRING  */
#line 875 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2865 "ncgeny.c"
    break;

  case 148: /* intlist: constint  */
#line 879 "ncgen.y"
                   {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2871 "ncgeny.c"
    break;

  case 149: /* intlist: intlist ',' constint  */
#line 880 "ncgen.y"
                               {(yyval.datalist)=(yyvsp[-2].datalist); dlappend((yyvsp[-2].datalist),((yyvsp[0].constant)));}
#line 2877 "ncgeny.c"
    break;

  case 150: /* constint: INT_CONST  */
#line 885 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_INT);}
#line 2883 "ncgeny.c"
    break;

  case 151: /* constint: UINT_CONST  */
#line 887 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2889 "ncgeny.c"
    break;

  case 152: /* constint: INT64_CONST  */
#line 889 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2895 "ncgeny.c"
    break;

  case 153: /* constint: UINT64_CONST  */
#line 891 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2901 "ncgeny.c"
    break;

  case 154: /* conststring: TERMSTRING  */
#line 895 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2907 "ncgeny.c"
    break;

  case 155: /* constbool: conststring  */
#line 899 "ncgen.y"
                      {(yyval.constant)=(yyvsp[0].constant);}
#line 2913 "ncgeny.c"
    break;

  case 156: /* constbool: constint  */
#line 900 "ncgen.y"
                   {(yyval.constant)=(yyvsp[0].constant);}
#line 2919 "ncgeny.c"
    break;

  case 157: /* varident: IDENT  */
#line 908 "ncgen.y"
                {(yyval.sym)=(yyvsp[0].sym);}
#line 2925 "ncgeny.c"
    break;

  case 158: /* varident: DATA  */
#line 909 "ncgen.y"
               {(yyval.sym)=identkeyword((yyvsp[0].sym));}
#line 2931 "ncgeny.c"
    break;

  case 159: /* ident: IDENT  */
#line 913 "ncgen.y"
              {(yyval.sym)=(yyvsp[0].sym);}
#line 2937 "ncgeny.c"
    break;


#line 2941 "ncgeny.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          goto yyexhaustedlab;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 916 "ncgen.y"


#ifndef NO_STDARG
static void
yyerror(const char *fmt, ...)
#else
static void
yyerror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    va_list argv;
    va_start(argv,fmt);
    (void)fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lineno);
    vderror(fmt,argv);
    va_end(argv);
}

/* undefine yywrap macro, in case we are using bison instead of yacc */
#ifdef yywrap
#undef yywrap
#endif

static int
ncgwrap(void)                    /* returns 1 on EOF if no more input */
{
    return  1;
}

/* get lexical input routine generated by lex  */
#include "ncgenl.c"

/* Really should init our data within this file */
void
parse_init(void)
{
    int i;
    opaqueid = 0;
    arrayuid = 0;
    symlist = listnew();
    stack = listnew();
    groupstack = listnew();
    consttype = NC_NAT;
    grpdefs = listnew();
    dimdefs = listnew();
    attdefs = listnew();
    gattdefs = listnew();
    xattdefs = listnew();
    typdefs = listnew();
    vardefs = listnew();
    tmp = listnew();
    /* Create the primitive types */
    for(i=NC_NAT+1;i<=NC_STRING;i++) {
        primsymbols[i] = makeprimitivetype(i);
    }
    lex_init();
}

static Symbol*
makeprimitivetype(nc_type nctype)
{
    Symbol* sym = install(primtypenames[nctype]);
    sym->objectclass=NC_TYPE;
    sym->subclass=NC_PRIM;
    sym->nc_id = nctype;
    sym->typ.typecode = nctype;
    sym->typ.size = ncsize(nctype);
    sym->typ.nelems = 1;
    (void)ncaux_class_alignment(nctype,&sym->typ.alignment);
    /* Make the basetype circular so we can always ask for it */
    sym->typ.basetype = sym;
    sym->prefix = listnew();
    return sym;
}

/* Symbol table operations for ncgen tool */
/* install sname in symbol table even if it is already there */
Symbol*
install(const char *sname)
{
    return installin(sname,currentgroup());
}

Symbol*
installin(const char *sname, Symbol* grp)
{
    Symbol* sp;
    sp = (Symbol*) ecalloc (sizeof (struct Symbol));
    sp->name = nulldup(sname);
    sp->lineno = lineno;
    sp->location = grp;
    sp->container = grp;
    listpush(symlist,sp);
    return sp;
}

static Symbol*
currentgroup(void)
{
    if(listlength(groupstack) == 0) return rootgroup;
    return (Symbol*)listtop(groupstack);
}

static Symbol*
createrootgroup(const char* dataset)
{
    Symbol* gsym = install(dataset);
    gsym->objectclass = NC_GRP;
    gsym->container = NULL;
    gsym->subnodes = listnew();
    gsym->grp.is_root = 1;
    gsym->prefix = listnew();
    listpush(grpdefs,(void*)gsym);
    rootgroup = gsym;
    return gsym;
}

static Symbol*
creategroup(Symbol * gsym)
{
    /* See if this group already exists in currentgroup */
    gsym->objectclass = NC_GRP;
    if(dupobjectcheck(NC_GRP,gsym)) {
        derror("Duplicate group name in same scope: %s",gsym->name);
	return NULL;
    }
    addtogroup(gsym);
    gsym->subnodes = listnew();
    listpush(groupstack,(void*)gsym);
    listpush(grpdefs,(void*)gsym);
    return gsym;
}

static NCConstant*
makeconstdata(nc_type nctype)
{
    NCConstant* con = nullconst();
    consttype = nctype;
    con->nctype = nctype;
    con->lineno = lineno;
    con->filled = 0;
    switch (nctype) {
	case NC_CHAR: con->value.charv = char_val; break;
        case NC_BYTE: con->value.int8v = byte_val; break;
        case NC_SHORT: con->value.int16v = int16_val; break;
        case NC_INT: con->value.int32v = int32_val; break;
        case NC_FLOAT:
	    con->value.floatv = float_val;
	    break;
        case NC_DOUBLE:
	    con->value.doublev = double_val;
	    break;
        case NC_STRING: { /* convert to a set of chars*/
	    size_t len;
	    len = bbLength(lextext);
	    con->value.stringv.len = len;
	    con->value.stringv.stringv = bbExtract(lextext);
	    }
	    break;

	/* Allow these constants even in netcdf-3 */
        case NC_UBYTE: con->value.uint8v = ubyte_val; break;
        case NC_USHORT: con->value.uint16v = uint16_val; break;
        case NC_UINT: con->value.uint32v = uint32_val; break;
        case NC_INT64: con->value.int64v = int64_val; break;
        case NC_UINT64: con->value.uint64v = uint64_val; break;

#ifdef USE_NETCDF4
	case NC_OPAQUE: {
	    char* s;
	    int len;
	    len = bbLength(lextext);
	    s = (char*)ecalloc(len+1);
	    strncpy(s,bbContents(lextext),len);
	    s[len] = '\0';
	    con->value.opaquev.stringv = s;
	    con->value.opaquev.len = len;
	    } break;

	case NC_NIL:
	    break; /* no associated value*/
#endif

 	case NC_FILLVALUE:
	    break; /* no associated value*/

	default:
	    yyerror("Data constant: unexpected NC type: %s",
		    nctypename(nctype));
	    con->value.stringv.stringv = NULL;
	    con->value.stringv.len = 0;
    }
    return con;
}

static NCConstant*
makeenumconstref(Symbol* refsym)
{
    NCConstant* con = nullconst();

    markcdf4("Enum type");
    consttype = NC_ENUM;
    con->nctype = NC_ECONST;
    con->lineno = lineno;
    con->filled = 0;
    refsym->objectclass = NC_TYPE;
    refsym->subclass = NC_ECONST;
    con->value.enumv = refsym;
    return con;
}

static void
addtogroup(Symbol* sym)
{
    Symbol* grp = currentgroup();
    sym->container = grp;
    listpush(grp->subnodes,(void*)sym);
    setpathcurrent(sym);
}

/* Check for duplicate name of given type within current group*/
static int
dupobjectcheck(nc_class objectclass, Symbol* pattern)
{
    int i;
    Symbol* grp;
    if(pattern == NULL) return 0;
    grp = pattern->container;
    if(grp == NULL || grp->subnodes == NULL) return 0;
    for(i=0;i<listlength(grp->subnodes);i++) {
	Symbol* sym = (Symbol*)listget(grp->subnodes,i);
	if(!sym->ref.is_ref && sym->objectclass == objectclass
	   && strcmp(sym->name,pattern->name)==0) return 1;
    }
    return 0;
}

static void
setpathcurrent(Symbol* sym)
{
    sym->is_prefixed = 0;
    sym->prefix = prefixdup(groupstack);
}

/* Convert an nc_type code to the corresponding Symbol*/
Symbol*
basetypefor(nc_type nctype)
{
    return primsymbols[nctype];
}

static int
truefalse(NCConstant* con, int tag)
{
    if(con->nctype == NC_STRING) {
	char* sdata = con->value.stringv.stringv;
	if(strncmp(sdata,"false",NC_MAX_NAME) == 0
           || strncmp(sdata,"0",NC_MAX_NAME) == 0)
	    return 0;
	else if(strncmp(sdata,"true",NC_MAX_NAME) == 0
           || strncmp(sdata,"1",NC_MAX_NAME) == 0)
	    return 1;
	else goto fail;
    } else if(con->value.int32v < 0 || con->value.int32v > 1)
	goto fail;
    return con->value.int32v;

fail:
    derror("%s: illegal value",specialname(tag));
    return 0;
}

/* Since this may be affected by the _Format attribute, which
   may come last, capture all the special info and sort it out
   in semantics.
*/
static Symbol*
makespecial(int tag, Symbol* vsym, Symbol* tsym, void* data, int isconst)
{
    Symbol* attr = NULL;
    Datalist* list = NULL;
    NCConstant* con = NULL;
    NCConstant* tmp = NULL;
    int tf = 0;
    char* sdata = NULL;
    int idata =  -1;

    if((GLOBAL_SPECIAL & tag) != 0) {
        if(vsym != NULL) {
            derror("_Format: must be global attribute");
            vsym = NULL;
        }
    } else {
        if(vsym == NULL) {
	    derror("%s: must have non-NULL vsym", specialname(tag));
	    return NULL;
        }
    }

    if(tag != _FILLVALUE_FLAG && tag != _FORMAT_FLAG)
        /*Main.*/specials_flag++;

    if(isconst)
	con = (NCConstant*)data;
    else
        list = (Datalist*)data;

    switch (tag) {
    case _FLETCHER32_FLAG:
    case _SHUFFLE_FLAG:
    case _ISNETCDF4_FLAG:
    case _NOFILL_FLAG:
	tmp = nullconst();
	tmp->nctype = (con->nctype == NC_STRING?NC_STRING:NC_INT);
	convert1(con,tmp);
	tf = truefalse(tmp,tag);
	reclaimconstant(tmp);
	break;
    case _FORMAT_FLAG:
    case _STORAGE_FLAG:
    case _NCPROPS_FLAG:
    case _ENDIAN_FLAG:
    case _FILTER_FLAG:
    case _CODECS_FLAG:
	tmp = nullconst();
        tmp->nctype = NC_STRING;
	convert1(con,tmp);
	if(tmp->nctype == NC_STRING) {
	    sdata = tmp->value.stringv.stringv;
	    tmp->value.stringv.stringv = NULL;
	    tmp->value.stringv.len = 0;
	} else
	    derror("%s: illegal value",specialname(tag));
	reclaimconstant(tmp);
	break;
    case _SUPERBLOCK_FLAG:
    case _DEFLATE_FLAG:
    case _QUANTIZEBG_FLAG:
    case _QUANTIZEGBR_FLAG:
    case _QUANTIZEBR_FLAG:
	tmp = nullconst();
        tmp->nctype = NC_INT;
	convert1(con,tmp);
	if(tmp->nctype == NC_INT)
	    idata = tmp->value.int32v;
	else
	    derror("%s: illegal value",specialname(tag));
	reclaimconstant(tmp);
	break;
    case _CHUNKSIZES_FLAG:
    case _FILLVALUE_FLAG:
	/* Handle below */
	break;
    default: PANIC1("unexpected special tag: %d",tag);
    }

    if(tag == _FORMAT_FLAG) {
	/* Watch out: this is a global attribute */
	struct Kvalues* kvalue;
	int found = 0;
	/* Use the table in main.c */
        for(kvalue = legalkinds; kvalue->name; kvalue++) {
          if(sdata) {
            if(strcmp(sdata, kvalue->name) == 0) {
              globalspecials._Format = kvalue->k_flag;
	      /*Main.*/format_attribute = 1;
              found = 1;
              break;
            }
          }
	}
	if(!found)
	    derror("_Format: illegal value: %s",sdata);
    } else if((GLOBAL_SPECIAL & tag) != 0) {
	if(tag == _ISNETCDF4_FLAG)
	    globalspecials._IsNetcdf4 = tf;
	else if(tag == _SUPERBLOCK_FLAG)
	    globalspecials._Superblock = idata;
	else if(tag == _NCPROPS_FLAG) {
	    globalspecials._NCProperties = sdata;
	    sdata = NULL;
	}
    } else {
        Specialdata* special;
        /* Set up special info */
        special = &vsym->var.special;
        if(tag == _FILLVALUE_FLAG) {
            /* fillvalue must be a single value*/
	    if(!isconst && datalistlen(list) != 1)
                derror("_FillValue: must be a single (possibly compound) value",
                            vsym->name);
	    if(isconst) {
	        list = const2list(con);
		con = NULL;
	    }
            /* check that the attribute value contains no fill values*/
            if(containsfills(list)) {
                derror("Attribute data may not contain fill values (i.e. _ )");
            }
            /* _FillValue is also a real attribute*/
            if(vsym->objectclass != NC_VAR) {
                derror("_FillValue attribute not associated with variable: %s",vsym->name);
            }
            if(tsym  == NULL) tsym = vsym->typ.basetype;
#if 0 /* No longer require matching types */
            else if(vsym->typ.basetype != tsym) {
                derror("_FillValue attribute type does not match variable type: %s",vsym->name);
            }
#endif
            special->_Fillvalue = clonedatalist(list);
	    /* Create the corresponding attribute */
            attr = makeattribute(install("_FillValue"),vsym,tsym,list,ATTRVAR);
	    list = NULL;
        } else switch (tag) {
	    /* These will be output as attributes later */
            case _STORAGE_FLAG:
              if(!sdata)
                derror("_Storage: illegal NULL value");
              else if(strcmp(sdata,"contiguous") == 0)
                special->_Storage = NC_CONTIGUOUS;
              else if(strcmp(sdata,"compact") == 0)
                special->_Storage = NC_COMPACT;
              else if(strcmp(sdata,"chunked") == 0)
                special->_Storage = NC_CHUNKED;
              else
                derror("_Storage: illegal value: %s",sdata);
              special->flags |= _STORAGE_FLAG;
              break;
          case _FLETCHER32_FLAG:
                special->_Fletcher32 = tf;
                special->flags |= _FLETCHER32_FLAG;
                break;
            case _DEFLATE_FLAG:
                special->_DeflateLevel = idata;
                special->flags |= _DEFLATE_FLAG;
                break;
            case _QUANTIZEBG_FLAG:
		special->_Quantizer = NC_QUANTIZE_BITGROOM;
                special->_NSD = idata;
                special->flags |= _QUANTIZEBG_FLAG;
                break;
            case _QUANTIZEGBR_FLAG:
		special->_Quantizer = NC_QUANTIZE_GRANULARBR;
                special->_NSD = idata;
                special->flags |= _QUANTIZEGBR_FLAG;
                break;
            case _QUANTIZEBR_FLAG:
		special->_Quantizer = NC_QUANTIZE_BITROUND;
                special->_NSD = idata;
                special->flags |= _QUANTIZEBR_FLAG;
                break;
            case _SHUFFLE_FLAG:
                special->_Shuffle = tf;
                special->flags |= _SHUFFLE_FLAG;
                break;
            case _ENDIAN_FLAG:
              if(!sdata)
                derror("_Endianness: illegal NULL value");
              else if(strcmp(sdata,"little") == 0)
                special->_Endianness = 1;
              else if(strcmp(sdata,"big") == 0)
                special->_Endianness = 2;
              else
                derror("_Endianness: illegal value: %s",sdata);
              special->flags |= _ENDIAN_FLAG;
              break;
          case _NOFILL_FLAG:
                special->_Fill = (1 - tf); /* negate */
                special->flags |= _NOFILL_FLAG;
                break;
          case _CHUNKSIZES_FLAG: {
                int i;
		list = (isconst ? const2list(con) : list);
                special->nchunks = list->length;
                special->_ChunkSizes = (size_t*)ecalloc(sizeof(size_t)*special->nchunks);
                for(i=0;i<special->nchunks;i++) {
		    tmp = nullconst();
                    tmp->nctype = NC_INT;
                    convert1(list->data[i],tmp);
                    if(tmp->nctype == NC_INT) {
                        special->_ChunkSizes[i] = (size_t)tmp->value.int32v;
                    } else {
                        efree(special->_ChunkSizes);
                        derror("%s: illegal value",specialname(tag));
                    }
		    reclaimconstant(tmp);
                }
                special->flags |= _CHUNKSIZES_FLAG;
                /* Chunksizes => storage == chunked */
                special->flags |= _STORAGE_FLAG;
                special->_Storage = NC_CHUNKED;
                } break;
          case _FILTER_FLAG:
#ifdef USE_NETCDF4
		/* Parse the filter spec */
		if(parsefilterflag(sdata,special) == NC_NOERR)
                    special->flags |= _FILTER_FLAG;
		else {
		    derror("_Filter: unparsable filter spec: %s",sdata);
		}
#else
	        derror("%s: the filter attribute requires netcdf-4 to be enabled",specialname(tag));
#endif
                break;
          case _CODECS_FLAG:
#ifdef USE_NETCDF4
		/* Parse the codec spec */
		if(parsecodecsflag(sdata,special) == NC_NOERR)
                    special->flags |= _CODECS_FLAG;
		else {
		    derror("_Codecs: unparsable codec spec: %s",sdata);
		}
#else
	        derror("%s: the _Codecs attribute requires netcdf-4 to be enabled",specialname(tag));
#endif
                break;
            default: PANIC1("makespecial: illegal token: %d",tag);
         }
    }
    if(sdata) efree(sdata);
    if(con) reclaimconstant(con);
    if(list) reclaimdatalist(list);
    return attr;
}

static Symbol*
makeattribute(Symbol* asym,
		Symbol* vsym,
		Symbol* tsym,
		Datalist* data,
		Attrkind kind) /* global var or unknown*/
{
    asym->objectclass = NC_ATT;
    asym->data = data;
    switch (kind) {
    case ATTRVAR:
        asym->att.var = vsym;
        asym->typ.basetype = tsym;
        listpush(attdefs,(void*)asym);
        addtogroup(asym);
	break;
    case ATTRGLOBAL:
        asym->att.var = NULL; /* NULL => NC_GLOBAL*/
        asym->typ.basetype = tsym;
        listpush(gattdefs,(void*)asym);
        addtogroup(asym);
	break;
    default: PANIC1("unexpected attribute type: %d",kind);
    }
    /* finally; check that the attribute value contains no fill values*/
    if(containsfills(data)) {
	derror("Attribute data may not contain fill values (i.e. _ ): %s",asym->name);
    }
    return asym;
}

static long long
extractint(NCConstant* con)
{
    switch (con->nctype) {
    case NC_BYTE: return (long long)(con->value.int8v);
    case NC_SHORT: return (long long)(con->value.int16v);
    case NC_INT: return (long long)(con->value.int32v);
    case NC_UBYTE: return (long long)(con->value.uint8v);
    case NC_USHORT: return (long long)(con->value.uint16v);
    case NC_UINT: return (long long)(con->value.uint32v);
    case NC_INT64: return (long long)(con->value.int64v);
    default:
	derror("Not a signed integer type: %d",con->nctype);
	break;
    }
    return 0;
}

static int
containsfills(Datalist* list)
{
    if(list != NULL) {
        int i;
        NCConstant** cons = list->data;
        for(i=0;i<list->length;i++) {
	    if(cons[i]->nctype == NC_COMPOUND) {
	        if(containsfills(cons[i]->value.compoundv)) return 1;
	    } else if(cons[i]->nctype == NC_FILLVALUE)
		return 1;
	}
    }
    return 0;
}

/*
Try to infer the file type from the
kinds of constructs used in the cdl file.
*/
static void
vercheck(int tid)
{
    switch (tid) {
    case NC_UBYTE: markcdf4("netCDF4/5 type: UBYTE"); break;
    case NC_USHORT: markcdf4("netCDF4/5 type: USHORT"); break;
    case NC_UINT: markcdf4("netCDF4/5 type: UINT"); break;
    case NC_INT64: markcdf4("netCDF4/5 type: INT64"); break;
    case NC_UINT64: markcdf4("netCDF4/5 type: UINT64"); break;
    case NC_STRING: markcdf4("netCDF4 type: STRING"); break;
    case NC_VLEN: markcdf4("netCDF4 type: VLEN"); break;
    case NC_OPAQUE: markcdf4("netCDF4 type: OPAQUE"); break;
    case NC_ENUM: markcdf4("netCDF4 type: ENUM"); break;
    case NC_COMPOUND: markcdf4("netCDF4 type: COMPOUND"); break;
    default: break;
    }
}

const char*
specialname(int tag)
{
    struct Specialtoken* spp = specials;
    for(;spp->name;spp++) {
	if(spp->tag == tag)
	    return spp->name;
    }
    return "<unknown>";
}

#ifdef USE_NETCDF4
/*
Parse a filter spec string and store it in special
*/
static int
parsefilterflag(const char* sdata, Specialdata* special)
{
    int stat = NC_NOERR;

    if(sdata == NULL || strlen(sdata) == 0) return NC_EINVAL;

    stat = ncaux_h5filterspec_parselist(sdata, NULL, &special->nfilters, &special->_Filters);
    if(stat)
        derror("Malformed filter spec: %s",sdata);
#ifdef GENDEBUG1
printfilters(special->nfilters,special->_Filters);
#endif
    return stat;
}

/*
Store a Codecs spec string in special
*/
static int
parsecodecsflag(const char* sdata, Specialdata* special)
{
    int stat = NC_NOERR;

    if(sdata == NULL || strlen(sdata) == 0) return NC_EINVAL;

    if((special->_Codecs = strdup(sdata))==NULL)
        return NC_ENOMEM;
    return stat;
}
#endif

/*
Since the arguments are all simple constants,
we can evaluate the function immediately
and return its value.
Note that currently, only a single value can
be returned.
*/

static NCConstant*
evaluate(Symbol* fcn, Datalist* arglist)
{
    NCConstant* result = nullconst();

    /* prepare the result */
    result->lineno = fcn->lineno;

    if(strcasecmp(fcn->name,"time") == 0) {
        char* timekind = NULL;
        char* timevalue = NULL;
        result->nctype = NC_DOUBLE;
        result->value.doublev = 0;
	/* int time([string],string) */
	switch (arglist->length) {
	case 2:
	    if(arglist->data[1]->nctype != NC_STRING) {
	        derror("Expected function signature: time([string,]string)");
	        goto done;
	    }
	    /* fall thru */
	case 1:
	    if(arglist->data[0]->nctype != NC_STRING) {
	        derror("Expected function signature: time([string,]string)");
	        goto done;
	    }
	    break;
	case 0:
	default:
	    derror("Expected function signature: time([string,]string)");
	    goto done;
	}
	if(arglist->length == 2) {
	    timekind = arglist->data[0]->value.stringv.stringv;
            timevalue = arglist->data[1]->value.stringv.stringv;
	} else
            timevalue = arglist->data[0]->value.stringv.stringv;
	if(timekind == NULL) { /* use cd time as the default */
            cdCompTime comptime;
	    CdTime cdtime;
	    cdCalenType timetype = cdStandard;
	    cdChar2Comp(timetype,timevalue,&comptime);
	    /* convert comptime to cdTime */
	    cdtime.year = comptime.year;
	    cdtime.month = comptime.month;
	    cdtime.day = comptime.day;
	    cdtime.hour = comptime.hour;
	    cdtime.baseYear = 1970;
	    cdtime.timeType = CdChron;
	    /* convert to double value */
	    Cdh2e(&cdtime,&result->value.doublev);
        } else {
	    derror("Time conversion '%s' not supported",timekind);
	    goto done;
	}
    } else {	/* Unknown function */
	derror("Unknown function name: %s",fcn->name);
	goto done;
    }

done:
    return result;
}

#ifdef GENDEBUG1
static void
printfilters(int nfilters, NC_FilterSpec** filters)
{
    int i;
    fprintf(stderr,"xxx: nfilters=%lu: ",(unsigned long)nfilters);
    for(i=0;i<nfilters;i++) {
	int k;
	NC_Filterspec* sp = filters[i];
        fprintf(stderr,"{");
        fprintf(stderr,"filterid=%llu format=%d nparams=%lu params=%p",
		sp->filterid,sp->format,(unsigned long)sp->nparams,sp->params);
	if(sp->nparams > 0 && sp->params != NULL) {
            fprintf(stderr," params={");
            for(k=0;k<sp->nparams;k++) {
	        if(i==0) fprintf(stderr,",");
	        fprintf(stderr,"%u",sp->params[i]);
	    }
            fprintf(stderr,"}");
	} else
            fprintf(stderr,"params=NULL");
        fprintf(stderr,"}");
    }
    fprintf(stderr,"\n");
    fflush(stderr);
}
#endif
