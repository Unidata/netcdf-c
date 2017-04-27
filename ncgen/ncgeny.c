/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

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

/* Copy the first part of user declarations.  */
#line 11 "ncgen.y" /* yacc.c:339  */

/*
static char SccsId[] = "$Id: ncgen.y,v 1.42 2010/05/18 21:32:46 dmh Exp $";
*/
#include        "includes.h"
#include        "ncoffsets.h"
#include        "ncgeny.h"
#include        "ncgen.h"

/* Following are in ncdump (for now)*/
/* Need some (unused) definitions to get it to compile */
#define ncatt_t void*
#define ncvar_t void
#include "nctime.h"

/* parser controls */
#define YY_NO_INPUT 1

/* True if string a equals string b*/
#ifndef STREQ
#define STREQ(a, b)     (*(a) == *(b) && strcmp((a), (b)) == 0)
#endif
#define VLENSIZE  (sizeof(nc_vlen_t))
#define MAXFLOATDIM 4294967295.0

/* mnemonic */
typedef enum Attrkind {ATTRVAR, ATTRGLOBAL, DONTKNOW} Attrkind;

typedef nc_vlen_t vlen_t;

/* We retain the old representation of the symbol list
   as a linked list.
*/
Symbol* symlist;

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
List* condefs; /* non-dimension constants used in type defs*/
List* tmp;

/* Forward */
static NCConstant makeconstdata(nc_type);
static NCConstant evaluate(Symbol* fcn, Datalist* arglist);
static NCConstant makeenumconstref(Symbol*);
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
static void datalistextend(Datalist* dl, NCConstant* con);
static void vercheck(int ncid);
static long long extractint(NCConstant con);

int yylex(void);

#ifndef NO_STDARG
static void yyerror(const char *fmt, ...);
#else
static void yyerror(fmt,va_alist) const char* fmt; va_dcl;
#endif

/* Extern */
extern int lex_init(void);


#line 202 "ncgeny.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "ncgeny.h".  */
#ifndef YY_NCG_NCGEN_TAB_H_INCLUDED
# define YY_NCG_NCGEN_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int ncgdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    _FILTERID = 309,
    _FILTERPARMS = 310,
    DATASETID = 311
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 141 "ncgen.y" /* yacc.c:355  */

Symbol* sym;
unsigned long  size; /* allow for zero size to indicate e.g. UNLIMITED*/
long           mark; /* track indices into the sequence*/
int            nctype; /* for tracking attribute list type*/
Datalist*      datalist;
NCConstant     constant;

#line 308 "ncgeny.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE ncglval;

int ncgparse (void);

#endif /* !YY_NCG_NCGEN_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 325 "ncgeny.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
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
#define YYLAST   424

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  66
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  67
/* YYNRULES -- Number of rules.  */
#define YYNRULES  154
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  265

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   311

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
static const yytype_uint16 yyrline[] =
{
       0,   224,   224,   230,   232,   239,   246,   246,   249,   258,
     248,   263,   264,   265,   269,   269,   271,   281,   281,   284,
     285,   286,   287,   290,   290,   293,   323,   325,   342,   351,
     363,   377,   410,   411,   414,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   442,   443,   444,
     447,   448,   451,   451,   453,   454,   458,   465,   475,   487,
     488,   489,   492,   493,   496,   496,   498,   520,   524,   528,
     555,   556,   559,   560,   564,   578,   582,   587,   616,   617,
     621,   622,   627,   637,   657,   668,   679,   698,   705,   705,
     708,   710,   712,   714,   716,   725,   736,   738,   740,   742,
     744,   746,   748,   750,   752,   754,   756,   758,   763,   770,
     779,   780,   781,   784,   785,   788,   792,   793,   797,   801,
     802,   807,   808,   812,   813,   814,   815,   816,   817,   821,
     825,   829,   831,   836,   837,   838,   839,   840,   841,   842,
     843,   844,   845,   846,   847,   851,   852,   856,   858,   860,
     862,   867,   871,   872,   878
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NC_UNLIMITED_K", "CHAR_K", "BYTE_K",
  "SHORT_K", "INT_K", "FLOAT_K", "DOUBLE_K", "UBYTE_K", "USHORT_K",
  "UINT_K", "INT64_K", "UINT64_K", "STRING_K", "IDENT", "TERMSTRING",
  "CHAR_CONST", "BYTE_CONST", "SHORT_CONST", "INT_CONST", "INT64_CONST",
  "UBYTE_CONST", "USHORT_CONST", "UINT_CONST", "UINT64_CONST",
  "FLOAT_CONST", "DOUBLE_CONST", "DIMENSIONS", "VARIABLES", "NETCDF",
  "DATA", "TYPES", "COMPOUND", "ENUM", "OPAQUE_", "OPAQUESTRING", "GROUP",
  "PATH", "FILLMARKER", "NIL", "_FILLVALUE", "_FORMAT", "_STORAGE",
  "_CHUNKSIZES", "_DEFLATELEVEL", "_SHUFFLE", "_ENDIANNESS", "_NOFILL",
  "_FLETCHER32", "_NCPROPS", "_ISNETCDF4", "_SUPERBLOCK", "_FILTERID",
  "_FILTERPARMS", "DATASETID", "'{'", "'}'", "';'", "','", "'='", "'('",
  "')'", "'*'", "':'", "$accept", "ncdesc", "datasetid", "rootgroup",
  "groupbody", "subgrouplist", "namedgroup", "$@1", "$@2", "typesection",
  "typedecls", "typename", "type_or_attr_decl", "typedecl", "optsemicolon",
  "enumdecl", "enumidlist", "enumid", "opaquedecl", "vlendecl",
  "compounddecl", "fields", "field", "primtype", "dimsection", "dimdecls",
  "dim_or_attr_decl", "dimdeclist", "dimdecl", "dimd", "vasection",
  "vadecls", "vadecl_or_attr", "vardecl", "varlist", "varspec", "dimspec",
  "dimlist", "dimref", "fieldlist", "fieldspec", "fielddimspec",
  "fielddimlist", "fielddim", "varref", "typeref", "type_var_ref",
  "attrdecllist", "attrdecl", "path", "datasection", "datadecls",
  "datadecl", "datalist", "datalist0", "datalist1", "dataitem",
  "constdata", "econstref", "function", "arglist", "simpleconstant",
  "intlist", "constint", "conststring", "constbool", "ident", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   123,   125,    59,
      44,    61,    40,    41,    42,    58
};
# endif

#define YYPACT_NINF -147

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-147)))

#define YYTABLE_NINF -109

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       6,   -14,    26,  -147,   -10,  -147,   229,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,    -8,  -147,  -147,   385,    -9,    28,     3,  -147,
    -147,     9,    14,    15,    19,    41,   -20,     4,   143,   216,
      60,   229,    77,    77,    65,     8,   347,    85,  -147,  -147,
      -1,    42,    43,    47,    50,    52,    55,    56,    59,    61,
      62,    63,    85,    45,   216,  -147,  -147,    66,    66,    66,
      66,    86,   291,    67,   229,    97,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,   347,  -147,    69,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,    73,    79,    76,    78,   347,    77,     8,
       8,    65,    77,    65,    65,     8,     8,   347,    81,  -147,
     119,  -147,  -147,  -147,  -147,  -147,  -147,    85,    80,  -147,
     229,    82,    83,  -147,    84,  -147,    88,   229,   114,    39,
     347,   239,  -147,   347,   347,    69,  -147,    90,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,    90,    69,   385,    91,    96,
      92,    99,  -147,    85,    46,   229,   101,  -147,   385,  -147,
     385,  -147,  -147,  -147,    -5,  -147,   229,    69,    69,     8,
     304,   102,    85,  -147,    85,    85,    85,  -147,  -147,  -147,
    -147,  -147,   103,  -147,   104,  -147,    12,   105,  -147,   385,
     108,   239,  -147,  -147,  -147,  -147,   109,  -147,   111,  -147,
     107,  -147,    40,  -147,   115,  -147,  -147,    85,    -7,  -147,
     347,   118,  -147,  -147,   140,  -147,    85,    -2,  -147,  -147,
      85,     8,  -147,   117,     0,  -147,  -147,    69,  -147,   122,
    -147,  -147,  -147,    33,  -147,  -147,  -147,    -7,  -147,   229,
      -2,  -147,  -147,  -147,  -147
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     3,     0,     1,    88,     2,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
     154,   109,     0,     6,    87,     0,    85,    11,     0,    86,
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
     102,   103,   104,   100,   105,   106,    95,     0,     0,     0,
       0,     0,    50,     0,     0,    61,     0,    64,     0,    65,
     111,     5,   122,   120,     0,   131,    88,    97,    94,     0,
       0,     0,     0,    85,     0,     0,     0,    51,    55,    58,
      57,    56,     0,    62,    66,    67,    70,     0,    84,   112,
       0,     0,   130,     6,   146,    31,     0,    32,    34,    75,
      78,    29,     0,    26,     0,    30,    63,     0,     0,    69,
     118,     0,   113,   132,     9,    33,     0,     0,    77,    25,
       0,     0,    68,    70,     0,    72,    74,   115,   114,     0,
      76,    83,    82,     0,    80,    27,    28,     0,    71,    88,
       0,    79,    73,    10,    81
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -147,  -147,  -147,  -147,    -4,   -32,  -147,  -147,  -147,  -147,
    -147,  -130,   131,  -147,   -48,  -147,  -147,   -57,  -147,  -147,
    -147,  -147,    11,   -25,  -147,  -147,    64,  -147,    23,  -147,
    -147,  -147,    24,  -147,  -147,   -24,  -147,  -147,   -55,  -147,
     -29,  -147,  -147,   -54,  -147,   -33,   -15,   -40,   -28,   -44,
    -147,  -147,     1,  -100,  -147,  -147,    98,  -147,  -147,  -147,
    -147,  -146,    89,   -41,   -30,   -73,   -22
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     4,     7,    23,    36,    49,   186,   249,    40,
      64,   128,    65,    66,   133,    67,   222,   223,    68,    69,
      70,   190,   191,    24,    75,   140,   141,   142,   143,   144,
     148,   175,   176,   177,   204,   205,   229,   244,   245,   218,
     219,   238,   253,   254,   207,    25,    26,    27,    28,    29,
     181,   209,   210,   105,   106,   107,   108,   109,   110,   111,
     184,   112,   157,    84,    85,    86,    30
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      35,    76,   104,   149,    87,   185,    72,   169,    20,    20,
      37,    73,    78,    79,    71,    20,    61,   155,    47,   251,
     134,   135,   136,   252,   113,   114,     5,   166,   116,    80,
      81,    72,    21,    82,    83,    31,    73,     1,    48,    71,
     129,   115,     3,    32,    33,    34,   145,     6,   160,   200,
     162,   163,   146,   187,   188,   211,    38,    37,   212,   104,
     257,    39,    41,   258,   221,   233,   225,    80,    81,    50,
      42,    82,    83,   104,   228,    43,    44,  -108,   158,   159,
      45,   113,    77,   104,   164,   158,    80,    81,   156,    74,
      82,    83,   161,   260,    77,   113,   261,   182,   239,   150,
     240,    20,    46,   117,   118,   113,   104,   130,   119,   104,
     104,   120,   145,   121,   178,   129,   122,   123,   146,   179,
     124,   137,   125,   126,   127,   132,   139,   147,   113,   150,
     247,   113,   113,   201,   192,   151,   152,   153,   167,   154,
     168,   172,   178,   173,   170,   174,   180,   179,   214,   -58,
     189,   199,   193,   195,   194,   196,   206,   192,   197,    20,
     203,   217,   226,    37,   227,   208,   230,   232,   235,   237,
     220,   236,   129,   224,   129,   193,   241,   248,    47,   228,
     259,   234,   213,   255,   246,    51,   104,    52,    53,    54,
      55,    56,    57,    58,   208,   131,   198,    59,    60,   202,
     256,   216,   262,   242,   171,   243,   264,   250,   113,     0,
     231,     0,     0,   246,   220,   165,     0,     0,   224,   263,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,     0,     0,   183,     0,
      62,     0,    63,     0,     0,    21,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    22,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   138,     0,     0,     0,     0,     0,     0,
       0,     0,   215,    20,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,     0,     0,     0,     0,
       0,     0,     0,     0,   100,     0,    21,   101,   102,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,     0,     0,   103,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    21
};

static const yytype_int16 yycheck[] =
{
      22,    41,    46,   103,    45,   151,    39,   137,    16,    16,
      25,    39,    42,    43,    39,    16,    38,   117,    38,    21,
      68,    69,    70,    25,    46,    47,     0,   127,    50,    21,
      22,    64,    39,    25,    26,    43,    64,    31,    58,    64,
      62,    42,    56,    51,    52,    53,    74,    57,   121,     3,
     123,   124,    74,   153,   154,    60,    65,    72,    63,   103,
      60,    33,    59,    63,   194,   211,   196,    21,    22,    65,
      61,    25,    26,   117,    62,    61,    61,    65,   119,   120,
      61,   103,    17,   127,   125,   126,    21,    22,   118,    29,
      25,    26,   122,    60,    17,   117,    63,    58,    58,    60,
      60,    16,    61,    61,    61,   127,   150,    62,    61,   153,
     154,    61,   140,    61,   147,   137,    61,    61,   140,   147,
      61,    35,    61,    61,    61,    59,    59,    30,   150,    60,
     230,   153,   154,   174,   167,    62,    57,    61,    57,    61,
      21,    59,   175,    60,    64,    61,    32,   175,   189,    61,
      60,   173,   167,    57,    63,    63,   178,   190,    59,    16,
      59,    59,    59,   178,    60,   180,    61,    59,    59,    62,
     192,    60,   194,   195,   196,   190,    61,    59,    38,    62,
      58,   213,   186,   240,   228,    42,   230,    44,    45,    46,
      47,    48,    49,    50,   209,    64,   173,    54,    55,   175,
     241,   190,   257,   227,   140,   227,   260,   236,   230,    -1,
     209,    -1,    -1,   257,   236,   126,    -1,    -1,   240,   259,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,   150,    -1,
      34,    -1,    36,    -1,    -1,    39,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    -1,    39,    40,    41,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    31,    67,    56,    68,     0,    57,    69,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    39,    65,    70,    89,   111,   112,   113,   114,   115,
     132,    43,    51,    52,    53,   132,    71,   112,    65,    33,
      75,    59,    61,    61,    61,    61,    61,    38,    58,    72,
      65,    42,    44,    45,    46,    47,    48,    49,    50,    54,
      55,   132,    34,    36,    76,    78,    79,    81,    84,    85,
      86,    89,   111,   114,    29,    90,   113,    17,   130,   130,
      21,    22,    25,    26,   129,   130,   131,   129,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      37,    40,    41,    57,   115,   119,   120,   121,   122,   123,
     124,   125,   127,   132,   132,    42,   132,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    77,   132,
      62,    78,    59,    80,    80,    80,    80,    35,    62,    59,
      91,    92,    93,    94,    95,   114,   132,    30,    96,   119,
      60,    62,    57,    61,    61,   119,   130,   128,   129,   129,
     131,   130,   131,   131,   129,   128,   119,    57,    21,    77,
      64,    92,    59,    60,    61,    97,    98,    99,   111,   114,
      32,   116,    58,   122,   126,   127,    73,   119,   119,    60,
      87,    88,   111,   112,    63,    57,    63,    59,    94,   132,
       3,   129,    98,    59,   100,   101,   132,   110,   112,   117,
     118,    60,    63,    70,   129,    58,    88,    59,   105,   106,
     132,    77,    82,    83,   132,    77,    59,    60,    62,   102,
      61,   118,    59,   127,    71,    59,    60,    62,   107,    58,
      60,    61,   101,   132,   103,   104,   115,   119,    59,    74,
     106,    21,    25,   108,   109,    83,   129,    60,    63,    58,
      60,    63,   104,   113,   109
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
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
     129,   130,   131,   131,   132
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
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
       1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
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
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
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
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
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
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
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
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
        case 2:
#line 227 "ncgen.y" /* yacc.c:1646  */
    {if (error_count > 0) YYABORT;}
#line 1643 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 3:
#line 230 "ncgen.y" /* yacc.c:1646  */
    {createrootgroup(datasetname);}
#line 1649 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 8:
#line 249 "ncgen.y" /* yacc.c:1646  */
    {
		Symbol* id = (yyvsp[-1].sym);
                markcdf4("Group specification");
		if(creategroup(id) == NULL)
                    yyerror("duplicate group declaration within parent group for %s",
                                id->name);
            }
#line 1661 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 9:
#line 258 "ncgen.y" /* yacc.c:1646  */
    {listpop(groupstack);}
#line 1667 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 12:
#line 264 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1673 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 13:
#line 266 "ncgen.y" /* yacc.c:1646  */
    {markcdf4("Type specification");}
#line 1679 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 16:
#line 272 "ncgen.y" /* yacc.c:1646  */
    { /* Use when defining a type */
              (yyvsp[0].sym)->objectclass = NC_TYPE;
              if(dupobjectcheck(NC_TYPE,(yyvsp[0].sym)))
                    yyerror("duplicate type declaration for %s",
                            (yyvsp[0].sym)->name);
              listpush(typdefs,(void*)(yyvsp[0].sym));
	    }
#line 1691 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 17:
#line 281 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1697 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 18:
#line 281 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1703 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 25:
#line 295 "ncgen.y" /* yacc.c:1646  */
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
#line 1734 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 26:
#line 324 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 1740 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 27:
#line 326 "ncgen.y" /* yacc.c:1646  */
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
#line 1759 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 28:
#line 343 "ncgen.y" /* yacc.c:1646  */
    {
            (yyvsp[-2].sym)->objectclass=NC_TYPE;
            (yyvsp[-2].sym)->subclass=NC_ECONST;
            (yyvsp[-2].sym)->typ.econst=(yyvsp[0].constant);
	    (yyval.sym)=(yyvsp[-2].sym);
        }
#line 1770 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 29:
#line 352 "ncgen.y" /* yacc.c:1646  */
    {
		    vercheck(NC_OPAQUE);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.typecode=NC_OPAQUE;
                    (yyvsp[0].sym)->typ.size=int32_val;
                    (yyvsp[0].sym)->typ.alignment=nctypealignment(NC_OPAQUE);
                }
#line 1784 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 30:
#line 364 "ncgen.y" /* yacc.c:1646  */
    {
                    Symbol* basetype = (yyvsp[-4].sym);
		    vercheck(NC_VLEN);
                    addtogroup((yyvsp[0].sym)); /*sets prefix*/
                    (yyvsp[0].sym)->objectclass=NC_TYPE;
                    (yyvsp[0].sym)->subclass=NC_VLEN;
                    (yyvsp[0].sym)->typ.basetype=basetype;
                    (yyvsp[0].sym)->typ.typecode=NC_VLEN;
                    (yyvsp[0].sym)->typ.size=VLENSIZE;
                    (yyvsp[0].sym)->typ.alignment=nctypealignment(NC_VLEN);
                }
#line 1800 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 31:
#line 378 "ncgen.y" /* yacc.c:1646  */
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
#line 1834 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 32:
#line 410 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-1].mark);}
#line 1840 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 33:
#line 411 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-2].mark);}
#line 1846 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 34:
#line 415 "ncgen.y" /* yacc.c:1646  */
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
#line 1862 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 35:
#line 428 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_CHAR]; }
#line 1868 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 36:
#line 429 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_BYTE]; }
#line 1874 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 37:
#line 430 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_SHORT]; }
#line 1880 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 38:
#line 431 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_INT]; }
#line 1886 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 39:
#line 432 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_FLOAT]; }
#line 1892 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 40:
#line 433 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym) = primsymbols[NC_DOUBLE]; }
#line 1898 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 41:
#line 434 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_UBYTE); (yyval.sym) = primsymbols[NC_UBYTE]; }
#line 1904 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 42:
#line 435 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_USHORT); (yyval.sym) = primsymbols[NC_USHORT]; }
#line 1910 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 43:
#line 436 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_UINT); (yyval.sym) = primsymbols[NC_UINT]; }
#line 1916 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 44:
#line 437 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_INT64); (yyval.sym) = primsymbols[NC_INT64]; }
#line 1922 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 45:
#line 438 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_UINT64); (yyval.sym) = primsymbols[NC_UINT64]; }
#line 1928 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 46:
#line 439 "ncgen.y" /* yacc.c:1646  */
    { vercheck(NC_STRING); (yyval.sym) = primsymbols[NC_STRING]; }
#line 1934 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 48:
#line 443 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1940 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 49:
#line 444 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1946 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 52:
#line 451 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1952 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 53:
#line 451 "ncgen.y" /* yacc.c:1646  */
    {}
#line 1958 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 56:
#line 459 "ncgen.y" /* yacc.c:1646  */
    {
		(yyvsp[-2].sym)->dim.declsize = (size_t)extractint((yyvsp[0].constant));
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = %llu\n",(yyvsp[-2].sym)->name,(unsigned long long)(yyvsp[-2].sym)->dim.declsize);
#endif
	      }
#line 1969 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 57:
#line 466 "ncgen.y" /* yacc.c:1646  */
    {
		        (yyvsp[-2].sym)->dim.declsize = NC_UNLIMITED;
		        (yyvsp[-2].sym)->dim.isunlimited = 1;
#ifdef GENDEBUG1
fprintf(stderr,"dimension: %s = UNLIMITED\n",(yyvsp[-2].sym)->name);
#endif
		   }
#line 1981 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 58:
#line 476 "ncgen.y" /* yacc.c:1646  */
    {
                     (yyvsp[0].sym)->objectclass=NC_DIM;
                     if(dupobjectcheck(NC_DIM,(yyvsp[0].sym)))
                        yyerror( "Duplicate dimension declaration for %s",
                                (yyvsp[0].sym)->name);
		     addtogroup((yyvsp[0].sym));
		     (yyval.sym)=(yyvsp[0].sym);
		     listpush(dimdefs,(void*)(yyvsp[0].sym));
                   }
#line 1995 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 60:
#line 488 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2001 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 61:
#line 489 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2007 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 64:
#line 496 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2013 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 65:
#line 496 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2019 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 66:
#line 499 "ncgen.y" /* yacc.c:1646  */
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
#line 2043 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 67:
#line 521 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack);
                 listpush(stack,(void*)(yyvsp[0].sym));
		}
#line 2051 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 68:
#line 525 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2057 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 69:
#line 529 "ncgen.y" /* yacc.c:1646  */
    {
		    int i;
		    Dimset dimset;
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
			(yyvsp[-1].sym)->typ.dimset = dimset;
		    }
		    (yyvsp[-1].sym)->typ.basetype = NULL; /* not yet known*/
                    (yyvsp[-1].sym)->objectclass=NC_VAR;
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		    }
#line 2086 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 70:
#line 555 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack);}
#line 2092 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 71:
#line 556 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-1].mark);}
#line 2098 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 72:
#line 559 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2104 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 73:
#line 561 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2110 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 74:
#line 565 "ncgen.y" /* yacc.c:1646  */
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
#line 2125 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 75:
#line 579 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack);
             listpush(stack,(void*)(yyvsp[0].sym));
	    }
#line 2133 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 76:
#line 583 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2139 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 77:
#line 588 "ncgen.y" /* yacc.c:1646  */
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
#line 2170 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 78:
#line 616 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack);}
#line 2176 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 79:
#line 617 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-1].mark);}
#line 2182 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 80:
#line 621 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2188 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 81:
#line 623 "ncgen.y" /* yacc.c:1646  */
    {(yyval.mark)=(yyvsp[-2].mark); listpush(stack,(void*)(yyvsp[0].sym));}
#line 2194 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 82:
#line 628 "ncgen.y" /* yacc.c:1646  */
    {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     sprintf(anon,"const%u",uint32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = uint32_val;
	    }
#line 2208 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 83:
#line 638 "ncgen.y" /* yacc.c:1646  */
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
#line 2226 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 84:
#line 658 "ncgen.y" /* yacc.c:1646  */
    {Symbol* vsym = (yyvsp[0].sym);
		if(vsym->objectclass != NC_VAR) {
		    derror("Undefined or forward referenced variable: %s",vsym->name);
		    YYABORT;
		}
		(yyval.sym)=vsym;
	    }
#line 2238 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 85:
#line 669 "ncgen.y" /* yacc.c:1646  */
    {Symbol* tsym = (yyvsp[0].sym);
		if(tsym->objectclass != NC_TYPE) {
		    derror("Undefined or forward referenced type: %s",tsym->name);
		    YYABORT;
		}
		(yyval.sym)=tsym;
	    }
#line 2250 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 86:
#line 680 "ncgen.y" /* yacc.c:1646  */
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
#line 2273 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 87:
#line 698 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym)=(yyvsp[0].sym);}
#line 2279 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 88:
#line 705 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2285 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 89:
#line 705 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2291 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 90:
#line 709 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_NCPROPS_FLAG,NULL,NULL,(void*)&(yyvsp[0].constant),ATTRGLOBAL);}
#line 2297 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 91:
#line 711 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_ISNETCDF4_FLAG,NULL,NULL,(void*)&(yyvsp[0].constant),ATTRGLOBAL);}
#line 2303 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 92:
#line 713 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_SUPERBLOCK_FLAG,NULL,NULL,(void*)&(yyvsp[0].constant),ATTRGLOBAL);}
#line 2309 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 93:
#line 715 "ncgen.y" /* yacc.c:1646  */
    { (yyval.sym)=makeattribute((yyvsp[-2].sym),NULL,NULL,(yyvsp[0].datalist),ATTRGLOBAL);}
#line 2315 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 94:
#line 717 "ncgen.y" /* yacc.c:1646  */
    {Symbol* tsym = (yyvsp[-5].sym); Symbol* vsym = (yyvsp[-4].sym); Symbol* asym = (yyvsp[-2].sym);
		if(vsym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,vsym,tsym,(yyvsp[0].datalist),ATTRVAR);
		} else {
		    derror("Doubly typed attribute: %s",asym->name);
		    YYABORT;
		}
	    }
#line 2328 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 95:
#line 726 "ncgen.y" /* yacc.c:1646  */
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
#line 2343 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 96:
#line 737 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),0);}
#line 2349 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 97:
#line 739 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[-4].sym),(yyvsp[-5].sym),(void*)(yyvsp[0].datalist),0);}
#line 2355 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 98:
#line 741 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_STORAGE_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2361 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 99:
#line 743 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_CHUNKSIZES_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),0);}
#line 2367 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 100:
#line 745 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FLETCHER32_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2373 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 101:
#line 747 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_DEFLATE_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2379 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 102:
#line 749 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_SHUFFLE_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2385 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 103:
#line 751 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_ENDIAN_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2391 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 104:
#line 753 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_NOFILL_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2397 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 105:
#line 755 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FILTERID_FLAG,(yyvsp[-4].sym),NULL,(void*)&(yyvsp[0].constant),1);}
#line 2403 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 106:
#line 757 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FILTERPARMS_FLAG,(yyvsp[-4].sym),NULL,(void*)(yyvsp[0].datalist),0);}
#line 2409 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 107:
#line 759 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym) = makespecial(_FORMAT_FLAG,NULL,NULL,(void*)&(yyvsp[0].constant),1);}
#line 2415 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 108:
#line 764 "ncgen.y" /* yacc.c:1646  */
    {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=0;
                setpathcurrent((yyvsp[0].sym));
	    }
#line 2426 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 109:
#line 771 "ncgen.y" /* yacc.c:1646  */
    {
	        (yyval.sym)=(yyvsp[0].sym);
                (yyvsp[0].sym)->ref.is_ref=1;
                (yyvsp[0].sym)->is_prefixed=1;
	        /* path is set in ncgen.l*/
	    }
#line 2437 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 111:
#line 780 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2443 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 112:
#line 781 "ncgen.y" /* yacc.c:1646  */
    {}
#line 2449 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 115:
#line 789 "ncgen.y" /* yacc.c:1646  */
    {(yyvsp[-2].sym)->data = (yyvsp[0].datalist);}
#line 2455 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 116:
#line 792 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2461 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 117:
#line 793 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = (yyvsp[0].datalist);}
#line 2467 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 118:
#line 797 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = builddatalist(0);}
#line 2473 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 119:
#line 801 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[0].constant)));}
#line 2479 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 120:
#line 803 "ncgen.y" /* yacc.c:1646  */
    {datalistextend((yyvsp[-2].datalist),&((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist);}
#line 2485 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 121:
#line 807 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=(yyvsp[0].constant);}
#line 2491 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 122:
#line 808 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=builddatasublist((yyvsp[-1].datalist));}
#line 2497 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 123:
#line 812 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=(yyvsp[0].constant);}
#line 2503 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 124:
#line 813 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_OPAQUE);}
#line 2509 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 125:
#line 814 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_FILLVALUE);}
#line 2515 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 126:
#line 815 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_NIL);}
#line 2521 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 127:
#line 816 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=(yyvsp[0].constant);}
#line 2527 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 129:
#line 821 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant) = makeenumconstref((yyvsp[0].sym));}
#line 2533 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 130:
#line 825 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=evaluate((yyvsp[-3].sym),(yyvsp[-1].datalist));}
#line 2539 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 131:
#line 830 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[0].constant)));}
#line 2545 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 132:
#line 832 "ncgen.y" /* yacc.c:1646  */
    {datalistextend((yyvsp[-2].datalist),&((yyvsp[0].constant))); (yyval.datalist)=(yyvsp[-2].datalist);}
#line 2551 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 133:
#line 836 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_CHAR);}
#line 2557 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 134:
#line 837 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_BYTE);}
#line 2563 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 135:
#line 838 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_SHORT);}
#line 2569 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 136:
#line 839 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_INT);}
#line 2575 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 137:
#line 840 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2581 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 138:
#line 841 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_UBYTE);}
#line 2587 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 139:
#line 842 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_USHORT);}
#line 2593 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 140:
#line 843 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2599 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 141:
#line 844 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2605 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 142:
#line 845 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_FLOAT);}
#line 2611 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 143:
#line 846 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_DOUBLE);}
#line 2617 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 144:
#line 847 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2623 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 145:
#line 851 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[0].constant)));}
#line 2629 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 146:
#line 852 "ncgen.y" /* yacc.c:1646  */
    {(yyval.datalist)=(yyvsp[-2].datalist); datalistextend((yyvsp[-2].datalist),&((yyvsp[0].constant)));}
#line 2635 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 147:
#line 857 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_INT);}
#line 2641 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 148:
#line 859 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_UINT);}
#line 2647 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 149:
#line 861 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_INT64);}
#line 2653 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 150:
#line 863 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_UINT64);}
#line 2659 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 151:
#line 867 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=makeconstdata(NC_STRING);}
#line 2665 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 152:
#line 871 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=(yyvsp[0].constant);}
#line 2671 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 153:
#line 872 "ncgen.y" /* yacc.c:1646  */
    {(yyval.constant)=(yyvsp[0].constant);}
#line 2677 "ncgeny.c" /* yacc.c:1646  */
    break;

  case 154:
#line 878 "ncgen.y" /* yacc.c:1646  */
    {(yyval.sym)=(yyvsp[0].sym);}
#line 2683 "ncgeny.c" /* yacc.c:1646  */
    break;


#line 2687 "ncgeny.c" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

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
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 881 "ncgen.y" /* yacc.c:1906  */


#ifndef NO_STDARG
static void
yyerror(const char *fmt, ...)
#else
static void
yyerror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    va_list argv;
    vastart(argv,fmt);
    (void)fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lineno);
    vderror(fmt,argv);
    vaend(argv,fmt);
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
    symlist = NULL;
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
    condefs = listnew();
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
    sym->ncid = nctype;
    sym->typ.typecode = nctype;
    sym->typ.size = ncsize(nctype);
    sym->typ.nelems = 1;
    sym->typ.alignment = nctypealignment(nctype);
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
    Symbol* sp;
    sp = (Symbol*) emalloc (sizeof (struct Symbol));
    memset((void*)sp,0,sizeof(struct Symbol));
    sp->name = nulldup(sname);
    sp->next = symlist;
    sp->lineno = lineno;
    sp->location = currentgroup();
    sp->container = currentgroup();
    symlist = sp;
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

static NCConstant
makeconstdata(nc_type nctype)
{
    NCConstant con = nullconstant;
    consttype = nctype;
    con.nctype = nctype;
    con.lineno = lineno;
    con.filled = 0;
    switch (nctype) {
	case NC_CHAR: con.value.charv = char_val; break;
        case NC_BYTE: con.value.int8v = byte_val; break;
        case NC_SHORT: con.value.int16v = int16_val; break;
        case NC_INT: con.value.int32v = int32_val; break;
        case NC_FLOAT:
	    con.value.floatv = float_val;
	    break;
        case NC_DOUBLE:
	    con.value.doublev = double_val;
	    break;
        case NC_STRING: { /* convert to a set of chars*/
	    size_t len;
	    len = bbLength(lextext);
	    con.value.stringv.len = len;
	    con.value.stringv.stringv = bbDup(lextext);
	    bbClear(lextext);
	    }
	    break;

	/* Allow these constants even in netcdf-3 */
        case NC_UBYTE: con.value.uint8v = ubyte_val; break;
        case NC_USHORT: con.value.uint16v = uint16_val; break;
        case NC_UINT: con.value.uint32v = uint32_val; break;
        case NC_INT64: con.value.int64v = int64_val; break;
        case NC_UINT64: con.value.uint64v = uint64_val; break;

#ifdef USE_NETCDF4
	case NC_OPAQUE: {
	    char* s;
	    int len;
	    len = bbLength(lextext);
	    s = (char*)emalloc(len+1);
	    strncpy(s,bbContents(lextext),len);
	    s[len] = '\0';
	    con.value.opaquev.stringv = s;
	    con.value.opaquev.len = len;
	    } break;

	case NC_NIL:
	    break; /* no associated value*/
#endif

	case NC_FILLVALUE:
	    break; /* no associated value*/

	default:
	    yyerror("Data constant: unexpected NC type: %s",
		    nctypename(nctype));
	    con.value.stringv.stringv = NULL;
	    con.value.stringv.len = 0;
    }
    return con;
}

static NCConstant
makeenumconstref(Symbol* refsym)
{
    NCConstant con;

    markcdf4("Enum type");
    consttype = NC_ENUM;
    con.nctype = NC_ECONST;
    con.lineno = lineno;
    con.filled = 0;
    refsym->objectclass = NC_TYPE;
    refsym->subclass = NC_ECONST;
    con.value.enumv = refsym;
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
    Datalist* list;
    NCConstant* con;
    NCConstant iconst;
    int tf = 0;
    char* sdata = NULL;
    int idata =  -1;
    unsigned int udata =  0;

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

    if(isconst) {
	con = (NCConstant*)data;
	list = builddatalist(1);
        dlappend(list,(NCConstant*)data);
    } else {
        list = (Datalist*)data;
        con = (NCConstant*)list->data;
    }

    switch (tag) {
    case _FLETCHER32_FLAG:
    case _SHUFFLE_FLAG:
    case _ISNETCDF4_FLAG:
    case _NOFILL_FLAG:
	iconst.nctype = (con->nctype == NC_STRING?NC_STRING:NC_INT);
	convert1(con,&iconst);
	tf = truefalse(&iconst,tag);
	break;
    case _FORMAT_FLAG:
    case _STORAGE_FLAG:
    case _NCPROPS_FLAG:
    case _ENDIAN_FLAG:
	iconst.nctype = NC_STRING;
	convert1(con,&iconst);
	if(iconst.nctype == NC_STRING)
	    sdata = iconst.value.stringv.stringv;
	else
	    derror("%s: illegal value",specialname(tag));
	break;
    case _SUPERBLOCK_FLAG:
    case _DEFLATE_FLAG:
	iconst.nctype = NC_INT;
	convert1(con,&iconst);
	if(iconst.nctype == NC_INT)
	    idata = iconst.value.int32v;
	else
	    derror("%s: illegal value",specialname(tag));
	break;
    case _CHUNKSIZES_FLAG:
    case _FILLVALUE_FLAG:
    case _FILTERPARMS_FLAG:
	/* Handle below */
	break;
    case _FILTERID_FLAG:
	iconst.nctype = NC_UINT;
	convert1(con,&iconst);
	if(iconst.nctype != NC_UINT) {
	     derror("%s: illegal filter id value",specialname(tag));
        }
	udata = iconst.value.uint32v;
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
	else if(tag == _NCPROPS_FLAG)
	    globalspecials._NCProperties = strdup(sdata);
    } else {
        Specialdata* special;
        /* Set up special info */
        special = &vsym->var.special;
        if(tag == _FILLVALUE_FLAG) {
            special->_Fillvalue = list;
            /* fillvalue must be a single value*/
            if(list->length != 1)
                derror("_FillValue: must be a single (possibly compound) value",
                            vsym->name);
            /* check that the attribute value contains no fill values*/
            if(containsfills(list)) {
                derror("Attribute data may not contain fill values (i.e. _ )");
            }
            /* _FillValue is also a real attribute*/
            if(vsym->objectclass != NC_VAR) {
                derror("_FillValue attribute not associated with variable: %s",vsym->name);
            }
            if(tsym  == NULL) tsym = vsym->typ.basetype;
            else if(vsym->typ.basetype != tsym) {
                derror("_FillValue attribute type does not match variable type: %s",vsym->name);
            }
            attr = makeattribute(install("_FillValue"),vsym,tsym,list,ATTRVAR);
        } else switch (tag) {
	    /* These will be output as attributes later */
            case _STORAGE_FLAG:
              if(!sdata)
                derror("_Storage: illegal NULL value");
              else if(strcmp(sdata,"contiguous") == 0)
                special->_Storage = NC_CONTIGUOUS;
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
                special->nchunks = list->length;
                special->_ChunkSizes = (size_t*)emalloc(sizeof(size_t)*special->nchunks);
                for(i=0;i<special->nchunks;i++) {
                    iconst.nctype = NC_INT;
                    convert1(&list->data[i],&iconst);
                    if(iconst.nctype == NC_INT) {
                        special->_ChunkSizes[i] = (size_t)iconst.value.int32v;
                    } else {
                        efree(special->_ChunkSizes);
                        derror("%s: illegal value",specialname(tag));
                    }
                }
                special->flags |= _CHUNKSIZES_FLAG;
                /* Chunksizes => storage == chunked */
                special->flags |= _STORAGE_FLAG;
                special->_Storage = NC_CHUNKED;
                } break;
          case _FILTERID_FLAG:
                special->_FilterID = udata;
                special->flags |= _FILTERID_FLAG;
                break;
          case _FILTERPARMS_FLAG: {
                int i;
                special->nparams = list->length;
                special->_FilterParams = (size_t*)emalloc(sizeof(size_t)*special->nparams);
                for(i=0;i<special->nparams;i++) {
                    iconst.nctype = NC_UINT;
                    convert1(&list->data[i],&iconst);
                    if(iconst.nctype == NC_UINT) {
                        special->_FilterParams[i] = iconst.value.uint32v;
                    } else {
                        efree(special->_FilterParams);
                        derror("%s: illegal filter parameter value",specialname(tag));
                    }
                }
                special->flags |= _FILTERPARMS_FLAG;
                } break;
            default: PANIC1("makespecial: illegal token: %d",tag);
         }
    }
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
extractint(NCConstant con)
{
    switch (con.nctype) {
    case NC_BYTE: return (long long)(con.value.int8v);
    case NC_SHORT: return (long long)(con.value.int16v);
    case NC_INT: return (long long)(con.value.int32v);
    case NC_UBYTE: return (long long)(con.value.uint8v);
    case NC_USHORT: return (long long)(con.value.uint16v);
    case NC_UINT: return (long long)(con.value.uint32v);
    case NC_INT64: return (long long)(con.value.int64v);
    default:
	derror("Not a signed integer type: %d",con.nctype);
	break;
    }
    return 0;
}

static int
containsfills(Datalist* list)
{
    if(list != NULL) {
        int i;
        NCConstant* con = list->data;
        for(i=0;i<list->length;i++,con++) {
	    if(con->nctype == NC_COMPOUND) {
	        if(containsfills(con->value.compoundv)) return 1;
	    } else if(con->nctype == NC_FILLVALUE) return 1;
	}
    }
    return 0;
}

static void
datalistextend(Datalist* dl, NCConstant* con)
{
    dlappend(dl,con);
}

static void
vercheck(int tid)
{
    switch (tid) {
    case NC_UBYTE: markcdf5("netCDF4/5 type: UBYTE"); break;
    case NC_USHORT: markcdf5("netCDF4/5 type: USHORT"); break;
    case NC_UINT: markcdf5("netCDF4/5 type: UINT"); break;
    case NC_INT64: markcdf5("netCDF4/5 type: INT64"); break;
    case NC_UINT64: markcdf5("netCDF4/5 type: UINT64"); break;
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

/*
Since the arguments are all simple constants,
we can evaluate the function immediately
and return its value.
Note that currently, only a single value can
be returned.
*/

static NCConstant
evaluate(Symbol* fcn, Datalist* arglist)
{
    NCConstant result = nullconstant;

    /* prepare the result */
    result.lineno = fcn->lineno;

    if(strcasecmp(fcn->name,"time") == 0) {
        char* timekind = NULL;
        char* timevalue = NULL;
        result.nctype = NC_DOUBLE;
        result.value.doublev = 0;
	/* int time([string],string) */
	switch (arglist->length) {
	case 2:
	    if(arglist->data[1].nctype != NC_STRING) {
	        derror("Expected function signature: time([string,]string)");
	        goto done;
	    }
	    /* fall thru */
	case 1:
	    if(arglist->data[0].nctype != NC_STRING) {
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
	    timekind = arglist->data[0].value.stringv.stringv;
            timevalue = arglist->data[1].value.stringv.stringv;
	} else
            timevalue = arglist->data[0].value.stringv.stringv;
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
	    Cdh2e(&cdtime,&result.value.doublev);
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
