/* A Bison parser, made by GNU Bison 3.8.2.  */

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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

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
  YYSYMBOL_DATASETID = 56,                 /* DATASETID  */
  YYSYMBOL_57_ = 57,                       /* '{'  */
  YYSYMBOL_58_ = 58,                       /* '}'  */
  YYSYMBOL_59_ = 59,                       /* ';'  */
  YYSYMBOL_60_ = 60,                       /* ','  */
  YYSYMBOL_61_ = 61,                       /* '='  */
  YYSYMBOL_62_ = 62,                       /* '('  */
  YYSYMBOL_63_ = 63,                       /* ')'  */
  YYSYMBOL_64_ = 64,                       /* '*'  */
  YYSYMBOL_65_ = 65,                       /* ':'  */
  YYSYMBOL_YYACCEPT = 66,                  /* $accept  */
  YYSYMBOL_ncdesc = 67,                    /* ncdesc  */
  YYSYMBOL_datasetid = 68,                 /* datasetid  */
  YYSYMBOL_rootgroup = 69,                 /* rootgroup  */
  YYSYMBOL_groupbody = 70,                 /* groupbody  */
  YYSYMBOL_subgrouplist = 71,              /* subgrouplist  */
  YYSYMBOL_namedgroup = 72,                /* namedgroup  */
  YYSYMBOL_73_1 = 73,                      /* $@1  */
  YYSYMBOL_74_2 = 74,                      /* $@2  */
  YYSYMBOL_typesection = 75,               /* typesection  */
  YYSYMBOL_typedecls = 76,                 /* typedecls  */
  YYSYMBOL_typename = 77,                  /* typename  */
  YYSYMBOL_type_or_attr_decl = 78,         /* type_or_attr_decl  */
  YYSYMBOL_typedecl = 79,                  /* typedecl  */
  YYSYMBOL_optsemicolon = 80,              /* optsemicolon  */
  YYSYMBOL_enumdecl = 81,                  /* enumdecl  */
  YYSYMBOL_enumidlist = 82,                /* enumidlist  */
  YYSYMBOL_enumid = 83,                    /* enumid  */
  YYSYMBOL_opaquedecl = 84,                /* opaquedecl  */
  YYSYMBOL_vlendecl = 85,                  /* vlendecl  */
  YYSYMBOL_compounddecl = 86,              /* compounddecl  */
  YYSYMBOL_fields = 87,                    /* fields  */
  YYSYMBOL_field = 88,                     /* field  */
  YYSYMBOL_primtype = 89,                  /* primtype  */
  YYSYMBOL_dimsection = 90,                /* dimsection  */
  YYSYMBOL_dimdecls = 91,                  /* dimdecls  */
  YYSYMBOL_dim_or_attr_decl = 92,          /* dim_or_attr_decl  */
  YYSYMBOL_dimdeclist = 93,                /* dimdeclist  */
  YYSYMBOL_dimdecl = 94,                   /* dimdecl  */
  YYSYMBOL_dimd = 95,                      /* dimd  */
  YYSYMBOL_vasection = 96,                 /* vasection  */
  YYSYMBOL_vadecls = 97,                   /* vadecls  */
  YYSYMBOL_vadecl_or_attr = 98,            /* vadecl_or_attr  */
  YYSYMBOL_vardecl = 99,                   /* vardecl  */
  YYSYMBOL_varlist = 100,                  /* varlist  */
  YYSYMBOL_varspec = 101,                  /* varspec  */
  YYSYMBOL_dimspec = 102,                  /* dimspec  */
  YYSYMBOL_dimlist = 103,                  /* dimlist  */
  YYSYMBOL_dimref = 104,                   /* dimref  */
  YYSYMBOL_fieldlist = 105,                /* fieldlist  */
  YYSYMBOL_fieldspec = 106,                /* fieldspec  */
  YYSYMBOL_fielddimspec = 107,             /* fielddimspec  */
  YYSYMBOL_fielddimlist = 108,             /* fielddimlist  */
  YYSYMBOL_fielddim = 109,                 /* fielddim  */
  YYSYMBOL_varref = 110,                   /* varref  */
  YYSYMBOL_typeref = 111,                  /* typeref  */
  YYSYMBOL_ambiguous_ref = 112,            /* ambiguous_ref  */
  YYSYMBOL_attrdecllist = 113,             /* attrdecllist  */
  YYSYMBOL_attrdecl = 114,                 /* attrdecl  */
  YYSYMBOL_path = 115,                     /* path  */
  YYSYMBOL_datasection = 116,              /* datasection  */
  YYSYMBOL_datadecls = 117,                /* datadecls  */
  YYSYMBOL_datadecl = 118,                 /* datadecl  */
  YYSYMBOL_datalist = 119,                 /* datalist  */
  YYSYMBOL_datalist0 = 120,                /* datalist0  */
  YYSYMBOL_datalist1 = 121,                /* datalist1  */
  YYSYMBOL_dataitem = 122,                 /* dataitem  */
  YYSYMBOL_constdata = 123,                /* constdata  */
  YYSYMBOL_econstref = 124,                /* econstref  */
  YYSYMBOL_function = 125,                 /* function  */
  YYSYMBOL_arglist = 126,                  /* arglist  */
  YYSYMBOL_simpleconstant = 127,           /* simpleconstant  */
  YYSYMBOL_intlist = 128,                  /* intlist  */
  YYSYMBOL_constint = 129,                 /* constint  */
  YYSYMBOL_conststring = 130,              /* conststring  */
  YYSYMBOL_constbool = 131,                /* constbool  */
  YYSYMBOL_varident = 132,                 /* varident  */
  YYSYMBOL_ident = 133                     /* ident  */
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

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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
#define YYLAST   462

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  267

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   311


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
      62,    63,    64,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    65,    59,
       2,    61,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,     2,    58,     2,     2,     2,     2,
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
      55,    56
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   240,   240,   246,   248,   255,   262,   262,   265,   274,
     264,   279,   280,   281,   285,   285,   287,   297,   297,   300,
     301,   302,   303,   306,   306,   309,   339,   341,   358,   367,
     379,   393,   426,   427,   430,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   458,   459,   460,
     463,   464,   467,   467,   469,   470,   474,   482,   492,   504,
     505,   506,   509,   510,   513,   513,   515,   537,   541,   545,
     574,   575,   578,   579,   583,   597,   601,   606,   635,   636,
     640,   641,   646,   656,   676,   687,   698,   717,   724,   724,
     727,   729,   731,   733,   735,   744,   755,   757,   759,   761,
     763,   765,   767,   769,   771,   773,   775,   777,   782,   789,
     798,   799,   800,   803,   804,   807,   811,   812,   816,   820,
     821,   826,   827,   831,   832,   833,   834,   835,   836,   840,
     844,   848,   850,   855,   856,   857,   858,   859,   860,   861,
     862,   863,   864,   865,   866,   870,   871,   875,   877,   879,
     881,   886,   890,   891,   899,   900,   904
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
  "_FILTER", "_CODECS", "DATASETID", "'{'", "'}'", "';'", "','", "'='",
  "'('", "')'", "'*'", "':'", "$accept", "ncdesc", "datasetid",
  "rootgroup", "groupbody", "subgrouplist", "namedgroup", "$@1", "$@2",
  "typesection", "typedecls", "typename", "type_or_attr_decl", "typedecl",
  "optsemicolon", "enumdecl", "enumidlist", "enumid", "opaquedecl",
  "vlendecl", "compounddecl", "fields", "field", "primtype", "dimsection",
  "dimdecls", "dim_or_attr_decl", "dimdeclist", "dimdecl", "dimd",
  "vasection", "vadecls", "vadecl_or_attr", "vardecl", "varlist",
  "varspec", "dimspec", "dimlist", "dimref", "fieldlist", "fieldspec",
  "fielddimspec", "fielddimlist", "fielddim", "varref", "typeref",
  "ambiguous_ref", "attrdecllist", "attrdecl", "path", "datasection",
  "datadecls", "datadecl", "datalist", "datalist0", "datalist1",
  "dataitem", "constdata", "econstref", "function", "arglist",
  "simpleconstant", "intlist", "constint", "conststring", "constbool",
  "varident", "ident", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-147)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-157)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -14,   -35,    41,  -147,   -27,  -147,   231,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,    -9,  -147,  -147,   423,   -18,    15,    -3,  -147,
    -147,     7,    25,    26,    32,    46,   -25,    -7,   145,   218,
      47,   231,    72,    72,     1,    49,   349,    74,  -147,  -147,
      -4,    50,    52,    55,    56,    59,    60,    61,    62,    63,
      64,    65,    74,    35,   218,  -147,  -147,    45,    45,    45,
      45,    67,   293,    68,   231,    78,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,   349,  -147,    69,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,    73,    79,    76,    77,   349,    72,    49,
      49,     1,    72,     1,     1,    72,    72,   349,    82,  -147,
     109,  -147,  -147,  -147,  -147,  -147,  -147,    74,    81,  -147,
     231,    84,    80,  -147,    85,  -147,    88,   231,   118,    36,
     349,   241,  -147,   349,   349,    69,  -147,    93,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,    69,   423,    91,    98,
      95,    97,  -147,    74,    29,   231,   100,  -147,   387,  -147,
     423,  -147,  -147,  -147,     9,  -147,   231,    69,    69,    49,
     306,   101,    74,  -147,    74,    74,    74,  -147,  -147,  -147,
    -147,  -147,   103,  -147,    99,  -147,   106,  -147,   105,   107,
    -147,   423,   110,   241,  -147,  -147,  -147,  -147,   112,  -147,
     116,  -147,   115,  -147,    43,  -147,   117,  -147,  -147,     3,
      -2,  -147,   349,   120,  -147,  -147,   142,  -147,    74,    28,
    -147,  -147,    74,    49,  -147,  -147,    17,  -147,  -147,    69,
    -147,    83,  -147,  -147,  -147,    22,  -147,  -147,  -147,    -2,
    -147,   231,    28,  -147,  -147,  -147,  -147
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     3,     0,     1,    88,     2,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
     156,   109,     0,     6,    87,     0,    85,    11,     0,    86,
     108,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      47,    88,     0,     0,     0,     0,   118,     0,     4,     7,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    14,    17,    23,    23,    23,
      23,    87,     0,     0,    48,    59,    89,   151,   107,    90,
     147,   149,   148,   150,   153,   152,    91,    92,   144,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     124,   125,   126,   118,   129,    93,   116,   117,   119,   121,
     127,   128,   123,   108,     0,     0,     0,   118,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   118,     0,    16,
       0,    15,    24,    19,    22,    21,    20,     0,     0,    18,
      49,     0,    52,    54,     0,    53,   108,    60,   110,     0,
       0,     0,     8,   118,   118,    96,    98,    99,   145,   101,
     102,   103,   106,   100,   104,   105,    95,     0,     0,     0,
       0,     0,    50,     0,     0,    61,     0,    64,     0,    65,
     111,     5,   122,   120,     0,   131,    88,    97,    94,     0,
       0,     0,     0,    85,     0,     0,     0,    51,    55,    58,
      57,    56,     0,    62,   154,   155,    66,    67,    70,     0,
      84,   112,     0,     0,   130,     6,   146,    31,     0,    32,
      34,    75,    78,    29,     0,    26,     0,    30,    63,     0,
       0,    69,   118,     0,   113,   132,     9,    33,     0,     0,
      77,    25,     0,     0,   154,    68,     0,    72,    74,   115,
     114,     0,    76,    83,    82,     0,    80,    27,    28,     0,
      71,    88,     0,    79,    73,    10,    81
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -147,  -147,  -147,  -147,    -5,   -32,  -147,  -147,  -147,  -147,
    -147,  -134,   121,  -147,    30,  -147,  -147,   -58,  -147,  -147,
    -147,  -147,    -8,   -19,  -147,  -147,    57,  -147,    31,  -147,
    -147,  -147,    23,  -147,  -147,   -26,  -147,  -147,   -54,  -147,
     -37,  -147,  -147,   -56,  -147,   -33,   -15,   -40,   -28,   -44,
    -147,  -147,     0,   -88,  -147,  -147,    58,  -147,  -147,  -147,
    -147,  -146,  -147,   -41,   -34,   -60,  -147,   -22
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     4,     7,    23,    36,    49,   186,   251,    40,
      64,   128,    65,    66,   133,    67,   224,   225,    68,    69,
      70,   190,   191,    24,    75,   140,   141,   142,   143,   144,
     148,   175,   176,   177,   206,   207,   231,   246,   247,   220,
     221,   240,   255,   256,   209,    25,    26,    27,    28,    29,
     181,   211,   212,   105,   106,   107,   108,   109,   110,   111,
     184,   112,   157,    84,    85,    86,   208,    30
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      35,    76,   104,   169,    87,   185,    72,    20,    78,    79,
      37,    73,    20,    47,    20,   149,    61,     1,    77,   244,
      71,     3,    80,    81,   113,   114,    82,    83,   116,   155,
       6,    72,   200,    48,    31,   205,    73,    21,   115,   166,
     129,     5,    32,    33,    34,    71,   145,    38,    39,   253,
      80,    81,   146,   254,    82,    83,    41,    37,    50,   104,
     223,   160,   227,   162,   163,   187,   188,   235,    42,   213,
      80,    81,   214,   104,    82,    83,    74,   259,   158,   159,
     260,   113,   262,   104,   156,   263,    43,    44,   161,    77,
      20,   164,   165,    45,   182,   113,   150,   130,   134,   135,
     136,   241,   137,   242,   132,   113,   104,    46,   147,   104,
     104,   117,   145,   118,   178,   129,   119,   120,   146,   179,
     121,   122,   123,   124,   125,   126,   127,   139,   113,   150,
     168,   113,   113,   201,   192,   151,   152,   153,   154,   167,
     173,   261,   178,   172,   249,   170,   174,   179,   216,   -58,
     180,   199,   193,   189,   194,   195,   197,   192,   196,   203,
     219,    20,   228,    37,  -156,   210,   229,   230,   232,   234,
     222,   237,   129,   226,   129,   193,   238,   239,   243,   250,
      47,   215,   218,   236,   257,   131,   248,    51,   104,    52,
      53,    54,    55,    56,    57,    58,   210,   171,   202,    59,
      60,   252,   258,   245,   198,   264,   266,     0,   183,     0,
     113,   233,     0,     0,     0,   248,   222,     0,     0,     0,
     226,   265,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     0,     0,
       0,     0,    62,     0,    63,     0,     0,    21,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   138,     0,     0,     0,     0,
       0,     0,     0,     0,   217,    20,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,     0,     0,
       0,     0,     0,     0,     0,     0,   100,     0,    21,   101,
     102,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,   204,     0,     0,   103,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,    21,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21
};

static const yytype_int16 yycheck[] =
{
      22,    41,    46,   137,    45,   151,    39,    16,    42,    43,
      25,    39,    16,    38,    16,   103,    38,    31,    17,    16,
      39,    56,    21,    22,    46,    47,    25,    26,    50,   117,
      57,    64,     3,    58,    43,    32,    64,    39,    42,   127,
      62,     0,    51,    52,    53,    64,    74,    65,    33,    21,
      21,    22,    74,    25,    25,    26,    59,    72,    65,   103,
     194,   121,   196,   123,   124,   153,   154,   213,    61,    60,
      21,    22,    63,   117,    25,    26,    29,    60,   119,   120,
      63,   103,    60,   127,   118,    63,    61,    61,   122,    17,
      16,   125,   126,    61,    58,   117,    60,    62,    68,    69,
      70,    58,    35,    60,    59,   127,   150,    61,    30,   153,
     154,    61,   140,    61,   147,   137,    61,    61,   140,   147,
      61,    61,    61,    61,    61,    61,    61,    59,   150,    60,
      21,   153,   154,   174,   167,    62,    57,    61,    61,    57,
      60,    58,   175,    59,   232,    64,    61,   175,   189,    61,
      32,   173,   167,    60,    63,    57,    59,   190,    63,    59,
      59,    16,    59,   178,    65,   180,    60,    62,    61,    59,
     192,    59,   194,   195,   196,   190,    60,    62,    61,    59,
      38,   186,   190,   215,   242,    64,   230,    42,   232,    44,
      45,    46,    47,    48,    49,    50,   211,   140,   175,    54,
      55,   238,   243,   229,   173,   259,   262,    -1,   150,    -1,
     232,   211,    -1,    -1,    -1,   259,   238,    -1,    -1,    -1,
     242,   261,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    -1,    -1,
      -1,    -1,    34,    -1,    36,    -1,    -1,    39,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,    39,    40,
      41,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    39,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    39
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    31,    67,    56,    68,     0,    57,    69,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    39,    65,    70,    89,   111,   112,   113,   114,   115,
     133,    43,    51,    52,    53,   133,    71,   112,    65,    33,
      75,    59,    61,    61,    61,    61,    61,    38,    58,    72,
      65,    42,    44,    45,    46,    47,    48,    49,    50,    54,
      55,   133,    34,    36,    76,    78,    79,    81,    84,    85,
      86,    89,   111,   114,    29,    90,   113,    17,   130,   130,
      21,    22,    25,    26,   129,   130,   131,   129,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      37,    40,    41,    57,   115,   119,   120,   121,   122,   123,
     124,   125,   127,   133,   133,    42,   133,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    77,   133,
      62,    78,    59,    80,    80,    80,    80,    35,    62,    59,
      91,    92,    93,    94,    95,   114,   133,    30,    96,   119,
      60,    62,    57,    61,    61,   119,   130,   128,   129,   129,
     131,   130,   131,   131,   130,   130,   119,    57,    21,    77,
      64,    92,    59,    60,    61,    97,    98,    99,   111,   114,
      32,   116,    58,   122,   126,   127,    73,   119,   119,    60,
      87,    88,   111,   112,    63,    57,    63,    59,    94,   133,
       3,   129,    98,    59,    16,    32,   100,   101,   132,   110,
     112,   117,   118,    60,    63,    70,   129,    58,    88,    59,
     105,   106,   133,    77,    82,    83,   133,    77,    59,    60,
      62,   102,    61,   118,    59,   127,    71,    59,    60,    62,
     107,    58,    60,    61,    16,   101,   103,   104,   115,   119,
      59,    74,   106,    21,    25,   108,   109,    83,   129,    60,
      63,    58,    60,    63,   104,   113,   109
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    66,    67,    68,    69,    70,    71,    71,    73,    74,
      72,    75,    75,    75,    76,    76,    77,    78,    78,    79,
      79,    79,    79,    80,    80,    81,    82,    82,    83,    84,
      85,    86,    87,    87,    88,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    90,    90,    90,
      91,    91,    92,    92,    93,    93,    94,    94,    95,    96,
      96,    96,    97,    97,    98,    98,    99,   100,   100,   101,
     102,   102,   103,   103,   104,   105,   105,   106,   107,   107,
     108,   108,   109,   109,   110,   111,   112,   112,   113,   113,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   115,   115,
     116,   116,   116,   117,   117,   118,   119,   119,   120,   121,
     121,   122,   122,   123,   123,   123,   123,   123,   123,   124,
     125,   126,   126,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   128,   128,   129,   129,   129,
     129,   130,   131,   131,   132,   132,   133
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
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
       5,     5,     5,     5,     5,     5,     5,     4,     1,     1,
       0,     1,     2,     2,     3,     3,     1,     1,     0,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
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
#line 243 "ncgen.y"
        {if (error_count > 0) YYABORT;}
#line 1853 "ncgeny.c"
    break;

  case 3: /* datasetid: DATASETID  */
#line 246 "ncgen.y"
                     {createrootgroup(datasetname);}
#line 1859 "ncgeny.c"
    break;

  case 8: /* $@1: %empty  */
#line 265 "ncgen.y"
            {
		Symbol* id = (yyvsp[-1].sym);
                markcdf4("Group specification");
		if(creategroup(id) == NULL)
                    yyerror("duplicate group declaration within parent group for %s",
                                id->name);
            }
#line 1871 "ncgeny.c"
    break;

  case 9: /* $@2: %empty  */
#line 274 "ncgen.y"
            {listpop(groupstack);}
#line 1877 "ncgeny.c"
    break;

  case 12: /* typesection: TYPES  */
#line 280 "ncgen.y"
                        {}
#line 1883 "ncgeny.c"
    break;

  case 13: /* typesection: TYPES typedecls  */
#line 282 "ncgen.y"
                        {markcdf4("Type specification");}
#line 1889 "ncgeny.c"
    break;

  case 16: /* typename: ident  */
#line 288 "ncgen.y"
            { /* Use when defining a type */
              (yyvsp[0].sym)->objectclass = NC_TYPE;
              if(dupobjectcheck(NC_TYPE,(yyvsp[0].sym)))
                    yyerror("duplicate type declaration for %s",
                            (yyvsp[0].sym)->name);
              listpush(typdefs,(void*)(yyvsp[0].sym));
	    }
#line 1901 "ncgeny.c"
    break;

  case 17: /* type_or_attr_decl: typedecl  */
#line 297 "ncgen.y"
                            {}
#line 1907 "ncgeny.c"
    break;

  case 18: /* type_or_attr_decl: attrdecl ';'  */
#line 297 "ncgen.y"
                                              {}
#line 1913 "ncgeny.c"
    break;

  case 25: /* enumdecl: primtype ENUM typename '{' enumidlist '}'  */
#line 311 "ncgen.y"
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
#line 1944 "ncgeny.c"
    break;

  case 26: /* enumidlist: enumid  */
#line 340 "ncgen.y"
                {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 1950 "ncgeny.c"
    break;

  case 27: /* enumidlist: enumidlist ',' enumid  */
#line 342 "ncgen.y"
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
#line 1969 "ncgeny.c"
    break;

  case 28: /* enumid: ident '=' constint  */
#line 359 "ncgen.y"
        {
            (yyvsp[-2].sym)->objectclass=NC_TYPE;
            (yyvsp[-2].sym)->subclass=NC_ECONST;
            (yyvsp[-2].sym)->typ.econst=(yyvsp[0].constant);
	    (yyval.sym)=(yyvsp[-2].sym);
        }
#line 1980 "ncgeny.c"
    break;

  case 29: /* opaquedecl: OPAQUE_ '(' INT_CONST ')' typename  */
#line 368 "ncgen.y"
                {
		    vercheck(NC_OPAQUE);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.typecode=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.size=int32_val;
                    (yyvsp[0].sym)->typ.alignment=ncaux_class_alignment(NC_OPAQUE);
                }
#line 1994 "ncgeny.c"
    break;

  case 30: /* vlendecl: typeref '(' '*' ')' typename  */
#line 380 "ncgen.y"
                {
                    Symbol* basetype = (yyvsp[-4].sym);
		    vercheck(NC_VLEN);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_VLEN;
                    (yyvsp[0].sym)->typ.basetype=basetype;
                    (yyvsp[0].sym)->typ.typecode=NC_VLEN;
                    (yyvsp[0].sym)->typ.size=VLENSIZE;
                    (yyvsp[0].sym)->typ.alignment=ncaux_class_alignment(NC_VLEN);
                }
#line 2010 "ncgeny.c"
    break;

  case 31: /* compounddecl: COMPOUND typename '{' fields '}'  */
#line 394 "ncgen.y"
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
#line 2044 "ncgeny.c"
    break;

  case 32: /* fields: field ';'  */
#line 426 "ncgen.y"
                    {(yyval.mark)=(yyvsp[-1].mark);}
#line 2050 "ncgeny.c"
    break;

  case 33: /* fields: fields field ';'  */
#line 427 "ncgen.y"
                              {(yyval.mark)=(yyvsp[-2].mark);}
#line 2056 "ncgeny.c"
    break;

  case 34: /* field: typeref fieldlist  */
#line 431 "ncgen.y"
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
#line 2072 "ncgeny.c"
    break;

  case 35: /* primtype: CHAR_K  */
#line 444 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_CHAR]; }
#line 2078 "ncgeny.c"
    break;

  case 36: /* primtype: BYTE_K  */
#line 445 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_BYTE]; }
#line 2084 "ncgeny.c"
    break;

  case 37: /* primtype: SHORT_K  */
#line 446 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_SHORT]; }
#line 2090 "ncgeny.c"
    break;

  case 38: /* primtype: INT_K  */
#line 447 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_INT]; }
#line 2096 "ncgeny.c"
    break;

  case 39: /* primtype: FLOAT_K  */
#line 448 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_FLOAT]; }
#line 2102 "ncgeny.c"
    break;

  case 40: /* primtype: DOUBLE_K  */
#line 449 "ncgen.y"
                          { (yyval.sym) = primsymbols[NC_DOUBLE]; }
#line 2108 "ncgeny.c"
    break;

  case 41: /* primtype: UBYTE_K  */
#line 450 "ncgen.y"
                           { vercheck(NC_UBYTE); (yyval.sym) = primsymbols[NC_UBYTE]; }
#line 2114 "ncgeny.c"
    break;

  case 42: /* primtype: USHORT_K  */
#line 451 "ncgen.y"
                           { vercheck(NC_USHORT); (yyval.sym) = primsymbols[NC_USHORT]; }
#line 2120 "ncgeny.c"
    break;

  case 43: /* primtype: UINT_K  */
#line 452 "ncgen.y"
                           { vercheck(NC_UINT); (yyval.sym) = primsymbols[NC_UINT]; }
#line 2126 "ncgeny.c"
    break;

  case 44: /* primtype: INT64_K  */
#line 453 "ncgen.y"
                            { vercheck(NC_INT64); (yyval.sym) = primsymbols[NC_INT64]; }
#line 2132 "ncgeny.c"
    break;

  case 45: /* primtype: UINT64_K  */
#line 454 "ncgen.y"
                             { vercheck(NC_UINT64); (yyval.sym) = primsymbols[NC_UINT64]; }
#line 2138 "ncgeny.c"
    break;

  case 46: /* primtype: STRING_K  */
#line 455 "ncgen.y"
                             { vercheck(NC_STRING); (yyval.sym) = primsymbols[NC_STRING]; }
#line 2144 "ncgeny.c"
    break;

  case 48: /* dimsection: DIMENSIONS  */
#line 459 "ncgen.y"
                             {}
#line 2150 "ncgeny.c"
    break;

  case 49: /* dimsection: DIMENSIONS dimdecls  */
#line 460 "ncgen.y"
                                      {}
#line 2156 "ncgeny.c"
    break;

  case 52: /* dim_or_attr_decl: dimdeclist  */
#line 467 "ncgen.y"
                             {}
#line 2162 "ncgeny.c"
    break;

  case 53: /* dim_or_attr_decl: attrdecl  */
#line 467 "ncgen.y"
                                           {}
#line 2168 "ncgeny.c"
    break;

  case 56: /* dimdecl: dimd '=' constint  */
#line 475 "ncgen.y"
              {
		(yyvsp[-2].sym)->dim.declsize = (size_t)extractint((yyvsp[0].constant));
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = %llu\n",(yyvsp[-2].sym)->name,(unsigned long long)(yyvsp[-2].sym)->dim.declsize);
#endif
		reclaimconstant((yyvsp[0].constant));
	      }
#line 2180 "ncgeny.c"
    break;

  case 57: /* dimdecl: dimd '=' NC_UNLIMITED_K  */
#line 483 "ncgen.y"
                   {
		        (yyvsp[-2].sym)->dim.declsize = NC_UNLIMITED;
		        (yyvsp[-2].sym)->dim.isunlimited = 1;
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = UNLIMITED\n",(yyvsp[-2].sym)->name);
#endif
		   }
#line 2192 "ncgeny.c"
    break;

  case 58: /* dimd: ident  */
#line 493 "ncgen.y"
                   {
                     (yyvsp[0].sym)->objectclass=NC_DIM;
                     if(dupobjectcheck(NC_DIM,(yyvsp[0].sym)))
                        yyerror( "Duplicate dimension declaration for %s",
                                (yyvsp[0].sym)->name);
		     addtogroup((yyvsp[0].sym));
		     (yyval.sym)=(yyvsp[0].sym);
		     listpush(dimdefs,(void*)(yyvsp[0].sym));
                   }
#line 2206 "ncgeny.c"
    break;

  case 60: /* vasection: VARIABLES  */
#line 505 "ncgen.y"
                            {}
#line 2212 "ncgeny.c"
    break;

  case 61: /* vasection: VARIABLES vadecls  */
#line 506 "ncgen.y"
                                    {}
#line 2218 "ncgeny.c"
    break;

  case 64: /* vadecl_or_attr: vardecl  */
#line 513 "ncgen.y"
                        {}
#line 2224 "ncgeny.c"
    break;

  case 65: /* vadecl_or_attr: attrdecl  */
#line 513 "ncgen.y"
                                      {}
#line 2230 "ncgeny.c"
    break;

  case 66: /* vardecl: typeref varlist  */
#line 516 "ncgen.y"
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
#line 2254 "ncgeny.c"
    break;

  case 67: /* varlist: varspec  */
#line 538 "ncgen.y"
                {(yyval.mark)=listlength(stack);
                 listpush(stack,(void*)(yyvsp[0].sym));
		}
#line 2262 "ncgeny.c"
    break;

  case 68: /* varlist: varlist ',' varspec  */
#line 542 "ncgen.y"
                {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2268 "ncgeny.c"
    break;

  case 69: /* varspec: varident dimspec  */
#line 546 "ncgen.y"
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
#line 2299 "ncgeny.c"
    break;

  case 70: /* dimspec: %empty  */
#line 574 "ncgen.y"
                            {(yyval.mark)=listlength(stack);}
#line 2305 "ncgeny.c"
    break;

  case 71: /* dimspec: '(' dimlist ')'  */
#line 575 "ncgen.y"
                                  {(yyval.mark)=(yyvsp[-1].mark);}
#line 2311 "ncgeny.c"
    break;

  case 72: /* dimlist: dimref  */
#line 578 "ncgen.y"
                       {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2317 "ncgeny.c"
    break;

  case 73: /* dimlist: dimlist ',' dimref  */
#line 580 "ncgen.y"
                    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2323 "ncgeny.c"
    break;

  case 74: /* dimref: path  */
#line 584 "ncgen.y"
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
#line 2338 "ncgeny.c"
    break;

  case 75: /* fieldlist: fieldspec  */
#line 598 "ncgen.y"
            {(yyval.mark)=listlength(stack);
             listpush(stack,(void*)(yyvsp[0].sym));
	    }
#line 2346 "ncgeny.c"
    break;

  case 76: /* fieldlist: fieldlist ',' fieldspec  */
#line 602 "ncgen.y"
            {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2352 "ncgeny.c"
    break;

  case 77: /* fieldspec: ident fielddimspec  */
#line 607 "ncgen.y"
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
#line 2383 "ncgeny.c"
    break;

  case 78: /* fielddimspec: %empty  */
#line 635 "ncgen.y"
                                 {(yyval.mark)=listlength(stack);}
#line 2389 "ncgeny.c"
    break;

  case 79: /* fielddimspec: '(' fielddimlist ')'  */
#line 636 "ncgen.y"
                                       {(yyval.mark)=(yyvsp[-1].mark);}
#line 2395 "ncgeny.c"
    break;

  case 80: /* fielddimlist: fielddim  */
#line 640 "ncgen.y"
                   {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2401 "ncgeny.c"
    break;

  case 81: /* fielddimlist: fielddimlist ',' fielddim  */
#line 642 "ncgen.y"
            {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2407 "ncgeny.c"
    break;

  case 82: /* fielddim: UINT_CONST  */
#line 647 "ncgen.y"
            {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     sprintf(anon,"const%u",uint32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = uint32_val;
	    }
#line 2421 "ncgeny.c"
    break;

  case 83: /* fielddim: INT_CONST  */
#line 657 "ncgen.y"
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
#line 2439 "ncgeny.c"
    break;

  case 84: /* varref: ambiguous_ref  */
#line 677 "ncgen.y"
            {Symbol* vsym = (yyvsp[0].sym);
		if(vsym->objectclass != NC_VAR) {
		    derror("Undefined or forward referenced variable: %s",vsym->name);
		    YYABORT;
		}
		(yyval.sym)=vsym;
	    }
#line 2451 "ncgeny.c"
    break;

  case 85: /* typeref: ambiguous_ref  */
#line 688 "ncgen.y"
            {Symbol* tsym = (yyvsp[0].sym);
		if(tsym->objectclass != NC_TYPE) {
		    derror("Undefined or forward referenced type: %s",tsym->name);
		    YYABORT;
		}
		(yyval.sym)=tsym;
	    }
#line 2463 "ncgeny.c"
    break;

  case 86: /* ambiguous_ref: path  */
#line 699 "ncgen.y"
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
#line 2486 "ncgeny.c"
    break;

  case 87: /* ambiguous_ref: primtype  */
#line 717 "ncgen.y"
                   {(yyval.sym)=(yyvsp[0].sym);}
#line 2492 "ncgeny.c"
    break;

  case 88: /* attrdecllist: %empty  */
#line 724 "ncgen.y"
                        {}
#line 2498 "ncgeny.c"
    break;

  case 89: /* attrdecllist: attrdecl ';' attrdecllist  */
#line 724 "ncgen.y"
                                                       {}
#line 2504 "ncgeny.c"
    break;

  case 90: /* attrdecl: ':' _NCPROPS '=' conststring  */
#line 728 "ncgen.y"
            {(yyval.sym) = makespecial(_NCPROPS_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2510 "ncgeny.c"
    break;

  case 91: /* attrdecl: ':' _ISNETCDF4 '=' constbool  */
#line 730 "ncgen.y"
            {(yyval.sym) = makespecial(_ISNETCDF4_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2516 "ncgeny.c"
    break;

  case 92: /* attrdecl: ':' _SUPERBLOCK '=' constint  */
#line 732 "ncgen.y"
            {(yyval.sym) = makespecial(_SUPERBLOCK_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2522 "ncgeny.c"
    break;

  case 93: /* attrdecl: ':' ident '=' datalist  */
#line 734 "ncgen.y"
            { (yyval.sym)=makeattribute((yyvsp[-2].sym),NULL,NULL,(yyvsp[0].datalist),ATTRGLOBAL);}
#line 2528 "ncgeny.c"
    break;

  case 94: /* attrdecl: typeref ambiguous_ref ':' ident '=' datalist  */
#line 736 "ncgen.y"
            {Symbol* tsym = (yyvsp[-5].sym); Symbol* vsym = (yyvsp[-4].sym); Symbol* asym = (yyvsp[-2].sym);
		if(vsym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,vsym,tsym,(yyvsp[0].datalist),ATTRVAR);
		} else {
		    derror("Doubly typed attribute: %s",asym->name);
		    YYABORT;
		}
	    }
#line 2541 "ncgeny.c"
    break;

  case 95: /* attrdecl: ambiguous_ref ':' ident '=' datalist  */
#line 745 "ncgen.y"
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
#line 2556 "ncgeny.c"
    break;

  case 96: /* attrdecl: ambiguous_ref ':' _FILLVALUE '=' datalist  */
#line 756 "ncgen.y"
            {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),ISLIST);}
#line 2562 "ncgeny.c"
    break;

  case 97: /* attrdecl: typeref ambiguous_ref ':' _FILLVALUE '=' datalist  */
#line 758 "ncgen.y"
            {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),(yyvsp[-5].sym),(void*)(yyvsp[0].datalist),ISLIST);}
#line 2568 "ncgeny.c"
    break;

  case 98: /* attrdecl: ambiguous_ref ':' _STORAGE '=' conststring  */
#line 760 "ncgen.y"
            {(yyval.sym) = makespecial(_STORAGE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2574 "ncgeny.c"
    break;

  case 99: /* attrdecl: ambiguous_ref ':' _CHUNKSIZES '=' intlist  */
#line 762 "ncgen.y"
            {(yyval.sym) = makespecial(_CHUNKSIZES_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),ISLIST);}
#line 2580 "ncgeny.c"
    break;

  case 100: /* attrdecl: ambiguous_ref ':' _FLETCHER32 '=' constbool  */
#line 764 "ncgen.y"
            {(yyval.sym) = makespecial(_FLETCHER32_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2586 "ncgeny.c"
    break;

  case 101: /* attrdecl: ambiguous_ref ':' _DEFLATELEVEL '=' constint  */
#line 766 "ncgen.y"
            {(yyval.sym) = makespecial(_DEFLATE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2592 "ncgeny.c"
    break;

  case 102: /* attrdecl: ambiguous_ref ':' _SHUFFLE '=' constbool  */
#line 768 "ncgen.y"
            {(yyval.sym) = makespecial(_SHUFFLE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2598 "ncgeny.c"
    break;

  case 103: /* attrdecl: ambiguous_ref ':' _ENDIANNESS '=' conststring  */
#line 770 "ncgen.y"
            {(yyval.sym) = makespecial(_ENDIAN_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2604 "ncgeny.c"
    break;

  case 104: /* attrdecl: ambiguous_ref ':' _FILTER '=' conststring  */
#line 772 "ncgen.y"
            {(yyval.sym) = makespecial(_FILTER_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2610 "ncgeny.c"
    break;

  case 105: /* attrdecl: ambiguous_ref ':' _CODECS '=' conststring  */
#line 774 "ncgen.y"
            {(yyval.sym) = makespecial(_CODECS_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2616 "ncgeny.c"
    break;

  case 106: /* attrdecl: ambiguous_ref ':' _NOFILL '=' constbool  */
#line 776 "ncgen.y"
            {(yyval.sym) = makespecial(_NOFILL_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2622 "ncgeny.c"
    break;

  case 107: /* attrdecl: ':' _FORMAT '=' conststring  */
#line 778 "ncgen.y"
            {(yyval.sym) = makespecial(_FORMAT_FLAG,NULL,NULL,(void*)(yyvsp[0].constant),ISCONST);}
#line 2628 "ncgeny.c"
    break;

  case 108: /* path: ident  */
#line 783 "ncgen.y"
            {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=0;
                setpathcurrent((yyvsp[0].sym));
	    }
#line 2639 "ncgeny.c"
    break;

  case 109: /* path: PATH  */
#line 790 "ncgen.y"
            {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=1;
	        /* path is set in ncgen.l*/
	    }
#line 2650 "ncgeny.c"
    break;

  case 111: /* datasection: DATA  */
#line 799 "ncgen.y"
                       {}
#line 2656 "ncgeny.c"
    break;

  case 112: /* datasection: DATA datadecls  */
#line 800 "ncgen.y"
                                 {}
#line 2662 "ncgeny.c"
    break;

  case 115: /* datadecl: varref '=' datalist  */
#line 808 "ncgen.y"
                   {(yyvsp[-2].sym)->data = (yyvsp[0].datalist);}
#line 2668 "ncgeny.c"
    break;

  case 116: /* datalist: datalist0  */
#line 811 "ncgen.y"
                    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2674 "ncgeny.c"
    break;

  case 117: /* datalist: datalist1  */
#line 812 "ncgen.y"
                    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2680 "ncgeny.c"
    break;

  case 118: /* datalist0: %empty  */
#line 816 "ncgen.y"
                  {(yyval.datalist) = builddatalist(0);}
#line 2686 "ncgeny.c"
    break;

  case 119: /* datalist1: dataitem  */
#line 820 "ncgen.y"
                   {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2692 "ncgeny.c"
    break;

  case 120: /* datalist1: datalist ',' dataitem  */
#line 822 "ncgen.y"
            {dlappend((yyvsp[-2].datalist),((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist); }
#line 2698 "ncgeny.c"
    break;

  case 121: /* dataitem: constdata  */
#line 826 "ncgen.y"
                    {(yyval.constant)=(yyvsp[0].constant);}
#line 2704 "ncgeny.c"
    break;

  case 122: /* dataitem: '{' datalist '}'  */
#line 827 "ncgen.y"
                           {(yyval.constant)=builddatasublist((yyvsp[-1].datalist));}
#line 2710 "ncgeny.c"
    break;

  case 123: /* constdata: simpleconstant  */
#line 831 "ncgen.y"
                              {(yyval.constant)=(yyvsp[0].constant);}
#line 2716 "ncgeny.c"
    break;

  case 124: /* constdata: OPAQUESTRING  */
#line 832 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_OPAQUE);}
#line 2722 "ncgeny.c"
    break;

  case 125: /* constdata: FILLMARKER  */
#line 833 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_FILLVALUE);}
#line 2728 "ncgeny.c"
    break;

  case 126: /* constdata: NIL  */
#line 834 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_NIL);}
#line 2734 "ncgeny.c"
    break;

  case 127: /* constdata: econstref  */
#line 835 "ncgen.y"
                        {(yyval.constant)=(yyvsp[0].constant);}
#line 2740 "ncgeny.c"
    break;

  case 129: /* econstref: path  */
#line 840 "ncgen.y"
             {(yyval.constant) = makeenumconstref((yyvsp[0].sym));}
#line 2746 "ncgeny.c"
    break;

  case 130: /* function: ident '(' arglist ')'  */
#line 844 "ncgen.y"
                              {(yyval.constant)=evaluate((yyvsp[-3].sym),(yyvsp[-1].datalist));}
#line 2752 "ncgeny.c"
    break;

  case 131: /* arglist: simpleconstant  */
#line 849 "ncgen.y"
            {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2758 "ncgeny.c"
    break;

  case 132: /* arglist: arglist ',' simpleconstant  */
#line 851 "ncgen.y"
            {dlappend((yyvsp[-2].datalist),((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist);}
#line 2764 "ncgeny.c"
    break;

  case 133: /* simpleconstant: CHAR_CONST  */
#line 855 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_CHAR);}
#line 2770 "ncgeny.c"
    break;

  case 134: /* simpleconstant: BYTE_CONST  */
#line 856 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_BYTE);}
#line 2776 "ncgeny.c"
    break;

  case 135: /* simpleconstant: SHORT_CONST  */
#line 857 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_SHORT);}
#line 2782 "ncgeny.c"
    break;

  case 136: /* simpleconstant: INT_CONST  */
#line 858 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_INT);}
#line 2788 "ncgeny.c"
    break;

  case 137: /* simpleconstant: INT64_CONST  */
#line 859 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2794 "ncgeny.c"
    break;

  case 138: /* simpleconstant: UBYTE_CONST  */
#line 860 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UBYTE);}
#line 2800 "ncgeny.c"
    break;

  case 139: /* simpleconstant: USHORT_CONST  */
#line 861 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_USHORT);}
#line 2806 "ncgeny.c"
    break;

  case 140: /* simpleconstant: UINT_CONST  */
#line 862 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2812 "ncgeny.c"
    break;

  case 141: /* simpleconstant: UINT64_CONST  */
#line 863 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2818 "ncgeny.c"
    break;

  case 142: /* simpleconstant: FLOAT_CONST  */
#line 864 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_FLOAT);}
#line 2824 "ncgeny.c"
    break;

  case 143: /* simpleconstant: DOUBLE_CONST  */
#line 865 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_DOUBLE);}
#line 2830 "ncgeny.c"
    break;

  case 144: /* simpleconstant: TERMSTRING  */
#line 866 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2836 "ncgeny.c"
    break;

  case 145: /* intlist: constint  */
#line 870 "ncgen.y"
                   {(yyval.datalist) = const2list((yyvsp[0].constant));}
#line 2842 "ncgeny.c"
    break;

  case 146: /* intlist: intlist ',' constint  */
#line 871 "ncgen.y"
                               {(yyval.datalist)=(yyvsp[-2].datalist); dlappend((yyvsp[-2].datalist),((yyvsp[0].constant)));}
#line 2848 "ncgeny.c"
    break;

  case 147: /* constint: INT_CONST  */
#line 876 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_INT);}
#line 2854 "ncgeny.c"
    break;

  case 148: /* constint: UINT_CONST  */
#line 878 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2860 "ncgeny.c"
    break;

  case 149: /* constint: INT64_CONST  */
#line 880 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2866 "ncgeny.c"
    break;

  case 150: /* constint: UINT64_CONST  */
#line 882 "ncgen.y"
                {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2872 "ncgeny.c"
    break;

  case 151: /* conststring: TERMSTRING  */
#line 886 "ncgen.y"
                        {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2878 "ncgeny.c"
    break;

  case 152: /* constbool: conststring  */
#line 890 "ncgen.y"
                      {(yyval.constant)=(yyvsp[0].constant);}
#line 2884 "ncgeny.c"
    break;

  case 153: /* constbool: constint  */
#line 891 "ncgen.y"
                   {(yyval.constant)=(yyvsp[0].constant);}
#line 2890 "ncgeny.c"
    break;

  case 154: /* varident: IDENT  */
#line 899 "ncgen.y"
                {(yyval.sym)=(yyvsp[0].sym);}
#line 2896 "ncgeny.c"
    break;

  case 155: /* varident: DATA  */
#line 900 "ncgen.y"
               {(yyval.sym)=identkeyword((yyvsp[0].sym));}
#line 2902 "ncgeny.c"
    break;

  case 156: /* ident: IDENT  */
#line 904 "ncgen.y"
              {(yyval.sym)=(yyvsp[0].sym);}
#line 2908 "ncgeny.c"
    break;


#line 2912 "ncgeny.c"

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
          YYNOMEM;
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
  ++yynerrs;

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
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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

#line 907 "ncgen.y"


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
    sym->typ.alignment = ncaux_class_alignment(nctype);
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
