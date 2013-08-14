/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.7.12-4996"

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
#define yylval          ncglval
#define yychar          ncgchar
#define yydebug         ncgdebug
#define yynerrs         ncgnerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 11 "ncgen.y"

/*
static char SccsId[] = "$Id: ncgen.y,v 1.42 2010/05/18 21:32:46 dmh Exp $";
*/
#include        "includes.h"
#include        "offsets.h"

/* Following are in ncdump (for now)*/
/* Need some (unused) definitions to get it to compile */
#define ncatt_t void*
#define ncvar_t void
#include "nctime.h"

/* parser controls */
#define YY_NO_INPUT 1

/* True if string a equals string b*/
#define STREQ(a, b)     (*(a) == *(b) && strcmp((a), (b)) == 0)
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
static NCConstant makeenumconst(Symbol*);
static void addtogroup(Symbol*);
static Symbol* currentgroup(void);
static Symbol* createrootgroup(void);
static Symbol* creategroup(Symbol*);
static int dupobjectcheck(nc_class,Symbol*);
static void setpathcurrent(Symbol* sym);
static Symbol* makeattribute(Symbol*,Symbol*,Symbol*,Datalist*,Attrkind);
static Symbol* makeprimitivetype(nc_type i);
static Symbol* makespecial(int tag, Symbol* vsym, Symbol* tsym, void* data, int isconst);
static int containsfills(Datalist* list);
static void datalistextend(Datalist* dl, NCConstant* con);
static void vercheck(int ncid);

int yylex(void);

#ifndef NO_STDARG
static void yyerror(const char *fmt, ...);
#else
static void yyerror(fmt,va_alist) const char* fmt; va_dcl;
#endif

/* Extern */
extern int lex_init(void);


/* Line 371 of yacc.c  */
#line 193 "ncgen.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
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
   by #include "ncgen.tab.h".  */
#ifndef YY_NCG_NCGEN_TAB_H_INCLUDED
# define YY_NCG_NCGEN_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int ncgdebug;
#endif

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
     IDENT = 270,
     TERMSTRING = 271,
     CHAR_CONST = 272,
     BYTE_CONST = 273,
     SHORT_CONST = 274,
     INT_CONST = 275,
     INT64_CONST = 276,
     UBYTE_CONST = 277,
     USHORT_CONST = 278,
     UINT_CONST = 279,
     UINT64_CONST = 280,
     FLOAT_CONST = 281,
     DOUBLE_CONST = 282,
     DIMENSIONS = 283,
     VARIABLES = 284,
     NETCDF = 285,
     DATA = 286,
     TYPES = 287,
     COMPOUND = 288,
     ENUM = 289,
     OPAQUE = 290,
     OPAQUESTRING = 291,
     GROUP = 292,
     PATH = 293,
     FILLMARKER = 294,
     NIL = 295,
     _FILLVALUE = 296,
     _FORMAT = 297,
     _STORAGE = 298,
     _CHUNKSIZES = 299,
     _DEFLATELEVEL = 300,
     _SHUFFLE = 301,
     _ENDIANNESS = 302,
     _NOFILL = 303,
     _FLETCHER32 = 304,
     DATASETID = 305
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 131 "ncgen.y"

Symbol* sym;
unsigned long  size; /* allow for zero size to indicate e.g. UNLIMITED*/
long           mark; /* track indices into the sequence*/
int            nctype; /* for tracking attribute list type*/
Datalist*      datalist;
NCConstant       constant;


/* Line 387 of yacc.c  */
#line 296 "ncgen.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE ncglval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int ncgparse (void *YYPARSE_PARAM);
#else
int ncgparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int ncgparse (void);
#else
int ncgparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_NCG_NCGEN_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 324 "ncgen.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif


/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

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
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   376

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  60
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  148
/* YYNRULES -- Number of states.  */
#define YYNSTATES  249

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   305

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      56,    57,    58,     2,    54,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,    53,
       2,    55,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    51,     2,    52,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,    12,    18,    19,    22,    23,    24,
      34,    35,    37,    40,    42,    45,    47,    49,    52,    55,
      58,    61,    64,    65,    67,    74,    76,    80,    84,    90,
      96,   102,   105,   109,   112,   114,   116,   118,   120,   122,
     124,   126,   128,   130,   132,   134,   135,   137,   140,   143,
     147,   149,   151,   153,   157,   161,   165,   169,   173,   175,
     176,   178,   181,   184,   188,   190,   192,   195,   197,   201,
     204,   205,   209,   211,   215,   217,   219,   223,   226,   227,
     231,   233,   237,   239,   241,   243,   245,   247,   249,   250,
     254,   259,   266,   272,   278,   285,   291,   297,   303,   309,
     315,   321,   327,   332,   334,   336,   337,   339,   342,   345,
     349,   353,   355,   357,   358,   360,   364,   366,   370,   372,
     374,   376,   378,   380,   382,   387,   389,   393,   395,   397,
     399,   401,   403,   405,   407,   409,   411,   413,   415,   417,
     419,   423,   425,   427,   429,   431,   433,   435,   437
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      61,     0,    -1,    30,    50,    62,    -1,    51,    63,    64,
      52,    -1,   106,    68,    83,    89,   109,    -1,    -1,    64,
      65,    -1,    -1,    -1,    37,   124,    51,    66,    63,    64,
      67,    52,   106,    -1,    -1,    32,    -1,    32,    69,    -1,
      71,    -1,    69,    71,    -1,   124,    -1,    72,    -1,   107,
      53,    -1,    74,    73,    -1,    79,    73,    -1,    78,    73,
      -1,    77,    73,    -1,    -1,    53,    -1,    82,    34,    70,
      51,    75,    52,    -1,    76,    -1,    75,    54,    76,    -1,
     124,    55,   116,    -1,    35,    56,    20,    57,    70,    -1,
     104,    56,    58,    57,    70,    -1,    33,    70,    51,    80,
      52,    -1,    81,    53,    -1,    80,    81,    53,    -1,   104,
      98,    -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,
       8,    -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,
      13,    -1,    14,    -1,    -1,    28,    -1,    28,    84,    -1,
      85,    53,    -1,    84,    85,    53,    -1,    86,    -1,   107,
      -1,    87,    -1,    86,    54,    87,    -1,    88,    55,    24,
      -1,    88,    55,    20,    -1,    88,    55,    27,    -1,    88,
      55,     3,    -1,   124,    -1,    -1,    29,    -1,    29,    90,
      -1,    91,    53,    -1,    90,    91,    53,    -1,    92,    -1,
     107,    -1,   104,    93,    -1,    94,    -1,    93,    54,    94,
      -1,   124,    95,    -1,    -1,    56,    96,    57,    -1,    97,
      -1,    96,    54,    97,    -1,   108,    -1,    99,    -1,    98,
      54,    99,    -1,   124,   100,    -1,    -1,    56,   101,    57,
      -1,   102,    -1,   101,    54,   102,    -1,    24,    -1,    20,
      -1,   105,    -1,   105,    -1,   108,    -1,    82,    -1,    -1,
     107,    53,   106,    -1,    59,   124,    55,   112,    -1,   104,
     105,    59,   124,    55,   112,    -1,   105,    59,   124,    55,
     112,    -1,   105,    59,    41,    55,   112,    -1,   104,   105,
      59,    41,    55,   112,    -1,   105,    59,    43,    55,   122,
      -1,   105,    59,    44,    55,   120,    -1,   105,    59,    49,
      55,   123,    -1,   105,    59,    45,    55,   121,    -1,   105,
      59,    46,    55,   123,    -1,   105,    59,    47,    55,   122,
      -1,   105,    59,    48,    55,   123,    -1,    59,    42,    55,
     122,    -1,   124,    -1,    38,    -1,    -1,    31,    -1,    31,
     110,    -1,   111,    53,    -1,   110,   111,    53,    -1,   103,
      55,   112,    -1,   113,    -1,   114,    -1,    -1,   115,    -1,
     112,    54,   115,    -1,   116,    -1,    51,   112,    52,    -1,
     119,    -1,    36,    -1,    39,    -1,    40,    -1,   108,    -1,
     117,    -1,   124,    56,   118,    57,    -1,   119,    -1,   118,
      54,   119,    -1,    17,    -1,    18,    -1,    19,    -1,    20,
      -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,
      -1,    26,    -1,    27,    -1,    16,    -1,   121,    -1,   120,
      54,   121,    -1,    20,    -1,    24,    -1,    21,    -1,    25,
      -1,    16,    -1,   122,    -1,   121,    -1,    15,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   208,   208,   214,   221,   228,   228,   231,   240,   230,
     245,   246,   247,   251,   251,   253,   263,   263,   266,   267,
     268,   269,   272,   272,   275,   305,   307,   324,   333,   345,
     359,   392,   393,   396,   410,   411,   412,   413,   414,   415,
     416,   417,   418,   419,   420,   423,   424,   425,   428,   429,
     432,   432,   434,   435,   439,   446,   457,   470,   480,   492,
     493,   494,   497,   498,   501,   501,   503,   525,   529,   533,
     560,   561,   564,   565,   569,   583,   587,   592,   621,   622,
     626,   627,   632,   642,   662,   673,   684,   703,   710,   710,
     713,   715,   724,   735,   737,   739,   741,   743,   745,   747,
     749,   751,   753,   758,   764,   773,   774,   775,   778,   779,
     782,   786,   787,   791,   795,   796,   801,   802,   806,   807,
     808,   809,   810,   811,   815,   819,   821,   826,   827,   828,
     829,   830,   831,   832,   833,   834,   835,   836,   837,   841,
     842,   846,   848,   850,   852,   857,   861,   862,   868
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NC_UNLIMITED_K", "CHAR_K", "BYTE_K",
  "SHORT_K", "INT_K", "FLOAT_K", "DOUBLE_K", "UBYTE_K", "USHORT_K",
  "UINT_K", "INT64_K", "UINT64_K", "IDENT", "TERMSTRING", "CHAR_CONST",
  "BYTE_CONST", "SHORT_CONST", "INT_CONST", "INT64_CONST", "UBYTE_CONST",
  "USHORT_CONST", "UINT_CONST", "UINT64_CONST", "FLOAT_CONST",
  "DOUBLE_CONST", "DIMENSIONS", "VARIABLES", "NETCDF", "DATA", "TYPES",
  "COMPOUND", "ENUM", "OPAQUE", "OPAQUESTRING", "GROUP", "PATH",
  "FILLMARKER", "NIL", "_FILLVALUE", "_FORMAT", "_STORAGE", "_CHUNKSIZES",
  "_DEFLATELEVEL", "_SHUFFLE", "_ENDIANNESS", "_NOFILL", "_FLETCHER32",
  "DATASETID", "'{'", "'}'", "';'", "','", "'='", "'('", "')'", "'*'",
  "':'", "$accept", "ncdesc", "rootgroup", "groupbody", "subgrouplist",
  "namedgroup", "$@1", "$@2", "typesection", "typedecls", "typename",
  "type_or_attr_decl", "typedecl", "optsemicolon", "enumdecl",
  "enumidlist", "enumid", "opaquedecl", "vlendecl", "compounddecl",
  "fields", "field", "primtype", "dimsection", "dimdecls",
  "dim_or_attr_decl", "dimdeclist", "dimdecl", "dimd", "vasection",
  "vadecls", "vadecl_or_attr", "vardecl", "varlist", "varspec", "dimspec",
  "dimlist", "dimref", "fieldlist", "fieldspec", "fielddimspec",
  "fielddimlist", "fielddim", "varref", "typeref", "type_var_ref",
  "attrdecllist", "attrdecl", "path", "datasection", "datadecls",
  "datadecl", "datalist", "datalist0", "datalist1", "dataitem",
  "constdata", "function", "arglist", "simpleconstant", "intlist",
  "constint", "conststring", "constbool", "ident", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   123,   125,    59,    44,    61,    40,    41,    42,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    60,    61,    62,    63,    64,    64,    66,    67,    65,
      68,    68,    68,    69,    69,    70,    71,    71,    72,    72,
      72,    72,    73,    73,    74,    75,    75,    76,    77,    78,
      79,    80,    80,    81,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    83,    83,    83,    84,    84,
      85,    85,    86,    86,    87,    87,    87,    87,    88,    89,
      89,    89,    90,    90,    91,    91,    92,    93,    93,    94,
      95,    95,    96,    96,    97,    98,    98,    99,   100,   100,
     101,   101,   102,   102,   103,   104,   105,   105,   106,   106,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   108,   108,   109,   109,   109,   110,   110,
     111,   112,   112,   113,   114,   114,   115,   115,   116,   116,
     116,   116,   116,   116,   117,   118,   118,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   120,
     120,   121,   121,   121,   121,   122,   123,   123,   124
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     4,     5,     0,     2,     0,     0,     9,
       0,     1,     2,     1,     2,     1,     1,     2,     2,     2,
       2,     2,     0,     1,     6,     1,     3,     3,     5,     5,
       5,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     2,     2,     3,
       1,     1,     1,     3,     3,     3,     3,     3,     1,     0,
       1,     2,     2,     3,     1,     1,     2,     1,     3,     2,
       0,     3,     1,     3,     1,     1,     3,     2,     0,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     0,     3,
       4,     6,     5,     5,     6,     5,     5,     5,     5,     5,
       5,     5,     4,     1,     1,     0,     1,     2,     2,     3,
       3,     1,     1,     0,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     4,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,    88,     2,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,   148,   104,
       0,     5,    87,     0,    85,    10,     0,    86,   103,     0,
       0,     0,     0,     0,    11,    45,    88,     0,   113,     0,
       3,     6,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,    16,    22,    22,    22,
      22,    87,     0,     0,    46,    59,    89,   145,   102,   138,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   119,   120,   121,   113,   122,    90,   111,   112,   114,
     116,   123,   118,   103,     0,     0,     0,   113,     0,     0,
       0,     0,     0,     0,     0,   113,     0,    15,     0,    14,
      23,    18,    21,    20,    19,     0,     0,    17,    47,     0,
      50,    52,     0,    51,   103,    60,   105,     0,     0,     0,
       7,   113,   113,    93,    95,   141,   143,   142,   144,    96,
     139,    98,   147,   146,    99,   100,   101,    97,    92,     0,
       0,     0,     0,     0,    48,     0,     0,    61,     0,    64,
       0,    65,   106,     4,   117,   115,     0,   125,    88,    94,
      91,     0,     0,     0,     0,    85,     0,     0,     0,    49,
      53,    58,    57,    55,    54,    56,     0,    62,    66,    67,
      70,     0,    84,   107,     0,     0,   124,     5,   140,    30,
       0,    31,    33,    75,    78,    28,     0,    25,     0,    29,
      63,     0,     0,    69,   113,     0,   108,   126,     8,    32,
       0,     0,    77,    24,     0,     0,    68,    70,     0,    72,
      74,   110,   109,     0,    76,    83,    82,     0,    80,    26,
      27,     0,    71,    88,     0,    79,    73,     9,    81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     6,    21,    31,    41,   168,   233,    35,    54,
     106,    55,    56,   111,    57,   206,   207,    58,    59,    60,
     172,   173,    22,    65,   118,   119,   120,   121,   122,   126,
     157,   158,   159,   188,   189,   213,   228,   229,   202,   203,
     222,   237,   238,   191,    23,    24,    25,    26,    27,   163,
     193,   194,    86,    87,    88,    89,    90,    91,   166,    92,
     139,   142,   143,   144,    28
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -128
static const yytype_int16 yypact[] =
{
      -5,   -12,    57,    12,  -128,   207,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,
      -3,  -128,  -128,   323,    14,    37,    26,  -128,  -128,    27,
      28,    -9,    32,     3,   110,    59,   207,    82,   275,    77,
    -128,  -128,     0,    52,    55,    56,    78,    80,    84,    89,
      91,    92,    77,    75,   110,  -128,  -128,    79,    79,    79,
      79,   103,   219,    97,   207,   122,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,
    -128,  -128,  -128,  -128,   275,  -128,    99,  -128,  -128,  -128,
    -128,  -128,  -128,    98,   105,   106,   107,   275,    82,    69,
      69,    60,    82,    60,    60,   275,   109,  -128,   143,  -128,
    -128,  -128,  -128,  -128,  -128,    77,   108,  -128,   207,   111,
     113,  -128,   115,  -128,   116,   207,   134,    54,   275,   349,
    -128,   275,   275,    99,  -128,  -128,  -128,  -128,  -128,   114,
    -128,  -128,  -128,  -128,  -128,  -128,  -128,  -128,    99,   323,
     117,   121,   118,   123,  -128,    77,    34,   207,   124,  -128,
     323,  -128,   323,  -128,  -128,  -128,   -22,  -128,   207,    99,
      99,    69,   272,   125,    77,  -128,    77,    77,    77,  -128,
    -128,  -128,  -128,  -128,  -128,  -128,   126,  -128,   128,  -128,
     -29,   129,  -128,   323,   127,   349,  -128,  -128,  -128,  -128,
     132,  -128,   133,  -128,   130,  -128,    76,  -128,   135,  -128,
    -128,    77,    -2,  -128,   275,   136,  -128,  -128,   151,  -128,
      77,     2,  -128,  -128,    77,   324,  -128,   137,     5,  -128,
    -128,    99,  -128,   139,  -128,  -128,  -128,    10,  -128,  -128,
    -128,    -2,  -128,   207,     2,  -128,  -128,  -128,  -128
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -128,  -128,  -128,    29,    -1,  -128,  -128,  -128,  -128,  -128,
    -106,   144,  -128,    45,  -128,  -128,   -25,  -128,  -128,  -128,
    -128,    30,     6,  -128,  -128,    85,  -128,    49,  -128,  -128,
    -128,    43,  -128,  -128,    24,  -128,  -128,   -33,  -128,   -11,
    -128,  -128,    -8,  -128,   -23,   -20,   -36,   -30,   -31,  -128,
    -128,    44,   -76,  -128,  -128,   112,    13,  -128,  -128,  -127,
    -128,   -94,   -27,   -87,   -19
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -104
static const yytype_int16 yytable[] =
{
      66,    30,   167,    32,    63,   140,   141,    85,   127,   151,
      68,    62,    18,    18,    51,    18,   146,   147,    18,    93,
      94,   133,   235,    96,    63,     1,   236,   212,    39,   148,
    -103,    62,   195,   107,   123,   196,    19,   182,     3,    29,
      61,    95,    32,    40,    43,   124,    44,    45,    46,    47,
      48,    49,    50,    85,   183,   169,   170,     4,   184,   241,
      61,   185,   242,     5,   244,    93,    85,   245,   217,    34,
     205,   134,   209,    33,    85,   145,    67,   198,    93,    36,
     135,   136,    37,    38,   137,   138,    93,    64,   123,   135,
     136,    42,    18,   137,   138,   161,   107,    85,    67,   124,
      85,    85,   160,   112,   113,   114,   164,    97,   128,    93,
      98,    99,    93,    93,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,   174,   161,   223,   175,
     224,   108,   110,   100,   160,   101,   181,   115,   231,   102,
      32,   190,   192,    52,   103,    53,   104,   105,    19,   174,
     117,   125,   175,   128,   129,   204,   130,   107,   208,   107,
     149,   131,   132,   150,   154,   162,   152,   155,   171,    20,
     156,   -58,   177,   192,   176,   178,   179,   187,   201,   210,
     216,   230,   211,    85,   214,   219,   221,   220,    39,   232,
     225,   243,   227,   212,    85,    93,   218,   197,   109,   239,
     186,   204,   200,   153,   180,   208,    93,   247,   246,   234,
     230,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,   226,   248,   215,   240,     0,
     165,     0,     0,     0,     0,    19,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,     0,     0,     0,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,   116,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,     0,     0,
      18,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
      19,    81,     0,    19,    82,    83,     0,     0,     0,     0,
       0,     0,     0,     0,   199,     0,    84,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    18,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,     0,     0,     0,     0,     0,     0,     0,     0,
      81,    19,    19,    82,    83,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-128)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      36,    20,   129,    23,    34,    99,   100,    38,    84,   115,
      37,    34,    15,    15,    33,    15,   103,   104,    15,    38,
      39,    97,    20,    42,    54,    30,    24,    56,    37,   105,
      59,    54,    54,    52,    64,    57,    38,     3,    50,    42,
      34,    41,    62,    52,    41,    64,    43,    44,    45,    46,
      47,    48,    49,    84,    20,   131,   132,     0,    24,    54,
      54,    27,    57,    51,    54,    84,    97,    57,   195,    32,
     176,    98,   178,    59,   105,   102,    16,   171,    97,    53,
      20,    21,    55,    55,    24,    25,   105,    28,   118,    20,
      21,    59,    15,    24,    25,   125,   115,   128,    16,   118,
     131,   132,   125,    58,    59,    60,    52,    55,    54,   128,
      55,    55,   131,   132,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,   149,   157,    52,   149,
      54,    56,    53,    55,   157,    55,   155,    34,   214,    55,
     160,   160,   162,    33,    55,    35,    55,    55,    38,   172,
      53,    29,   172,    54,    56,   174,    51,   176,   177,   178,
      51,    55,    55,    20,    53,    31,    58,    54,    54,    59,
      55,    55,    51,   193,    57,    57,    53,    53,    53,    53,
      53,   212,    54,   214,    55,    53,    56,    54,    37,    53,
      55,    52,   211,    56,   225,   214,   197,   168,    54,   224,
     157,   220,   172,   118,   155,   224,   225,   243,   241,   220,
     241,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,   211,   244,   193,   225,    -1,
     128,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    59,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    56,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    36,    -1,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    -1,    51,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    38,    38,    39,    40,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    30,    61,    50,     0,    51,    62,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    38,
      59,    63,    82,   104,   105,   106,   107,   108,   124,    42,
     124,    64,   105,    59,    32,    68,    53,    55,    55,    37,
      52,    65,    59,    41,    43,    44,    45,    46,    47,    48,
      49,   124,    33,    35,    69,    71,    72,    74,    77,    78,
      79,    82,   104,   107,    28,    83,   106,    16,   122,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    36,    39,    40,    51,   108,   112,   113,   114,   115,
     116,   117,   119,   124,   124,    41,   124,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    70,   124,    56,    71,
      53,    73,    73,    73,    73,    34,    56,    53,    84,    85,
      86,    87,    88,   107,   124,    29,    89,   112,    54,    56,
      51,    55,    55,   112,   122,    20,    21,    24,    25,   120,
     121,   121,   121,   122,   123,   122,   123,   123,   112,    51,
      20,    70,    58,    85,    53,    54,    55,    90,    91,    92,
     104,   107,    31,   109,    52,   115,   118,   119,    66,   112,
     112,    54,    80,    81,   104,   105,    57,    51,    57,    53,
      87,   124,     3,    20,    24,    27,    91,    53,    93,    94,
     124,   103,   105,   110,   111,    54,    57,    63,   121,    52,
      81,    53,    98,    99,   124,    70,    75,    76,   124,    70,
      53,    54,    56,    95,    55,   111,    53,   119,    64,    53,
      54,    56,   100,    52,    54,    55,    94,   124,    96,    97,
     108,   112,    53,    67,    99,    20,    24,   101,   102,    76,
     116,    54,    57,    52,    54,    57,    97,   106,   102
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
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

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YYUSE (yytype);
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

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
      yychar = YYLEX;
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
     `$$ = $1'.

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
/* Line 1787 of yacc.c  */
#line 211 "ncgen.y"
    {if (error_count > 0) YYABORT;}
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 231 "ncgen.y"
    {
		Symbol* id = (yyvsp[(2) - (3)].sym);
                markcdf4("Group specification");
		if(creategroup(id) == NULL) 
                    yyerror("duplicate group declaration within parent group for %s",
                                id->name);
            }
    break;

  case 8:
/* Line 1787 of yacc.c  */
#line 240 "ncgen.y"
    {listpop(groupstack);}
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 246 "ncgen.y"
    {}
    break;

  case 12:
/* Line 1787 of yacc.c  */
#line 248 "ncgen.y"
    {markcdf4("Type specification");}
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 254 "ncgen.y"
    { /* Use when defining a type */
              (yyvsp[(1) - (1)].sym)->objectclass = NC_TYPE;
              if(dupobjectcheck(NC_TYPE,(yyvsp[(1) - (1)].sym)))
                    yyerror("duplicate type declaration for %s",
                            (yyvsp[(1) - (1)].sym)->name);
              listpush(typdefs,(void*)(yyvsp[(1) - (1)].sym));
	    }
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 263 "ncgen.y"
    {}
    break;

  case 17:
/* Line 1787 of yacc.c  */
#line 263 "ncgen.y"
    {}
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 277 "ncgen.y"
    {
		int i;
                addtogroup((yyvsp[(3) - (6)].sym)); /* sets prefix*/
                (yyvsp[(3) - (6)].sym)->objectclass=NC_TYPE;
                (yyvsp[(3) - (6)].sym)->subclass=NC_ENUM;
                (yyvsp[(3) - (6)].sym)->typ.basetype=(yyvsp[(1) - (6)].sym);
                (yyvsp[(3) - (6)].sym)->typ.size = (yyvsp[(1) - (6)].sym)->typ.size;
                (yyvsp[(3) - (6)].sym)->typ.alignment = (yyvsp[(1) - (6)].sym)->typ.alignment;
                stackbase=(yyvsp[(5) - (6)].mark);
                stacklen=listlength(stack);
                (yyvsp[(3) - (6)].sym)->subnodes = listnew();
                /* Variety of field fixups*/
		/* 1. add in the enum values*/
		/* 2. make this type be their container*/
		/* 3. make constant names visible in the group*/
		/* 4. set field basetype to be same as enum basetype*/
                for(i=stackbase;i<stacklen;i++) {
                   Symbol* eid = (Symbol*)listget(stack,i);
		   assert(eid->subclass == NC_ECONST);
		   addtogroup(eid);
                   listpush((yyvsp[(3) - (6)].sym)->subnodes,(void*)eid);
                   eid->container = (yyvsp[(3) - (6)].sym);
		   eid->typ.basetype = (yyvsp[(3) - (6)].sym)->typ.basetype;
                }               
                listsetlength(stack,stackbase);/* remove stack nodes*/
              }
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 306 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[(1) - (1)].sym));}
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 308 "ncgen.y"
    {
		    int i;
		    (yyval.mark)=(yyvsp[(1) - (3)].mark);
		    /* check for duplicates*/
		    stackbase=(yyvsp[(1) - (3)].mark);
		    stacklen=listlength(stack);
		    for(i=stackbase;i<stacklen;i++) {
		      Symbol* elem = (Symbol*)listget(stack,i);
		      if(strcmp((yyvsp[(3) - (3)].sym)->name,elem->name)==0)
  	                yyerror("duplicate enum declaration for %s",
        	                 elem->name);
		    }    	    
		    listpush(stack,(void*)(yyvsp[(3) - (3)].sym));
		}
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 325 "ncgen.y"
    {
            (yyvsp[(1) - (3)].sym)->objectclass=NC_TYPE;
            (yyvsp[(1) - (3)].sym)->subclass=NC_ECONST;
            (yyvsp[(1) - (3)].sym)->typ.econst=(yyvsp[(3) - (3)].constant);
	    (yyval.sym)=(yyvsp[(1) - (3)].sym);
        }
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 334 "ncgen.y"
    {
		    vercheck(NC_OPAQUE);
                    addtogroup((yyvsp[(5) - (5)].sym)); /*sets prefix*/
                    (yyvsp[(5) - (5)].sym)->objectclass=NC_TYPE;
                    (yyvsp[(5) - (5)].sym)->subclass=NC_OPAQUE;
                    (yyvsp[(5) - (5)].sym)->typ.typecode=NC_OPAQUE;
                    (yyvsp[(5) - (5)].sym)->typ.size=int32_val;
                    (yyvsp[(5) - (5)].sym)->typ.alignment=nctypealignment(NC_OPAQUE);
                }
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 346 "ncgen.y"
    {
                    Symbol* basetype = (yyvsp[(1) - (5)].sym);
		    vercheck(NC_VLEN);
                    addtogroup((yyvsp[(5) - (5)].sym)); /*sets prefix*/
                    (yyvsp[(5) - (5)].sym)->objectclass=NC_TYPE;
                    (yyvsp[(5) - (5)].sym)->subclass=NC_VLEN;
                    (yyvsp[(5) - (5)].sym)->typ.basetype=basetype;
                    (yyvsp[(5) - (5)].sym)->typ.typecode=NC_VLEN;
                    (yyvsp[(5) - (5)].sym)->typ.size=VLENSIZE;
                    (yyvsp[(5) - (5)].sym)->typ.alignment=nctypealignment(NC_VLEN);
                }
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 360 "ncgen.y"
    {
	    int i,j;
	    vercheck(NC_COMPOUND);
            addtogroup((yyvsp[(2) - (5)].sym));
	    /* check for duplicate field names*/
	    stackbase=(yyvsp[(4) - (5)].mark);
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
	    (yyvsp[(2) - (5)].sym)->objectclass=NC_TYPE;
            (yyvsp[(2) - (5)].sym)->subclass=NC_COMPOUND;
            (yyvsp[(2) - (5)].sym)->typ.basetype=NULL;
            (yyvsp[(2) - (5)].sym)->typ.typecode=NC_COMPOUND;
	    (yyvsp[(2) - (5)].sym)->subnodes = listnew();
	    /* Add in the fields*/
	    for(i=stackbase;i<stacklen;i++) {
	        Symbol* fsym = (Symbol*)listget(stack,i);
		fsym->container = (yyvsp[(2) - (5)].sym);
 	        listpush((yyvsp[(2) - (5)].sym)->subnodes,(void*)fsym);
	    }    	    
	    listsetlength(stack,stackbase);/* remove stack nodes*/
          }
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 392 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (2)].mark);}
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 393 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark);}
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 397 "ncgen.y"
    {
	    int i;
	    (yyval.mark)=(yyvsp[(2) - (2)].mark);
	    stackbase=(yyvsp[(2) - (2)].mark);
	    stacklen=listlength(stack);
	    /* process each field in the fieldlist*/
            for(i=stackbase;i<stacklen;i++) {
                Symbol* f = (Symbol*)listget(stack,i);
		f->typ.basetype = (yyvsp[(1) - (2)].sym);
            }
        }
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 410 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_CHAR]; }
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 411 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_BYTE]; }
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 412 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_SHORT]; }
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 413 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_INT]; }
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 414 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_FLOAT]; }
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 415 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_DOUBLE]; }
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 416 "ncgen.y"
    { vercheck(NC_UBYTE); (yyval.sym) = primsymbols[NC_UBYTE]; }
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 417 "ncgen.y"
    { vercheck(NC_USHORT); (yyval.sym) = primsymbols[NC_USHORT]; }
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 418 "ncgen.y"
    { vercheck(NC_UINT); (yyval.sym) = primsymbols[NC_UINT]; }
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 419 "ncgen.y"
    { vercheck(NC_INT64); (yyval.sym) = primsymbols[NC_INT64]; }
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 420 "ncgen.y"
    { vercheck(NC_UINT64); (yyval.sym) = primsymbols[NC_UINT64]; }
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 424 "ncgen.y"
    {}
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 425 "ncgen.y"
    {}
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 432 "ncgen.y"
    {}
    break;

  case 51:
/* Line 1787 of yacc.c  */
#line 432 "ncgen.y"
    {}
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 440 "ncgen.y"
    {
		(yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)uint32_val;
#ifdef DEBUG1
fprintf(stderr,"dimension: %s = %lu\n",(yyvsp[(1) - (3)].sym)->name,(unsigned long)(yyvsp[(1) - (3)].sym)->dim.declsize);
#endif
	      }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 447 "ncgen.y"
    {
		if(int32_val <= 0) {
		    derror("dimension size must be positive");
		    YYABORT;
		}
		(yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)int32_val;
#ifdef DEBUG1
fprintf(stderr,"dimension: %s = %lu\n",(yyvsp[(1) - (3)].sym)->name,(unsigned long)(yyvsp[(1) - (3)].sym)->dim.declsize);
#endif
	      }
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 458 "ncgen.y"
    { /* for rare case where 2^31 < dimsize < 2^32 */
                       if (double_val <= 0)
                         yyerror("dimension length must be positive");
                       if (double_val > MAXFLOATDIM)
                         yyerror("dimension too large");
                       if (double_val - (size_t) double_val > 0)
                         yyerror("dimension length must be an integer");
                       (yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)double_val;
#ifdef DEBUG1
fprintf(stderr,"dimension: %s = %lu\n",(yyvsp[(1) - (3)].sym)->name,(unsigned long)(yyvsp[(1) - (3)].sym)->dim.declsize);
#endif
                   }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 471 "ncgen.y"
    {
		        (yyvsp[(1) - (3)].sym)->dim.declsize = NC_UNLIMITED;
		        (yyvsp[(1) - (3)].sym)->dim.isunlimited = 1;
#ifdef DEBUG1
fprintf(stderr,"dimension: %s = UNLIMITED\n",(yyvsp[(1) - (3)].sym)->name);
#endif
		   }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 481 "ncgen.y"
    { 
                     (yyvsp[(1) - (1)].sym)->objectclass=NC_DIM;
                     if(dupobjectcheck(NC_DIM,(yyvsp[(1) - (1)].sym)))
                        yyerror( "Duplicate dimension declaration for %s",
                                (yyvsp[(1) - (1)].sym)->name);
		     addtogroup((yyvsp[(1) - (1)].sym));
		     (yyval.sym)=(yyvsp[(1) - (1)].sym);
		     listpush(dimdefs,(void*)(yyvsp[(1) - (1)].sym));
                   }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 493 "ncgen.y"
    {}
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 494 "ncgen.y"
    {}
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 501 "ncgen.y"
    {}
    break;

  case 65:
/* Line 1787 of yacc.c  */
#line 501 "ncgen.y"
    {}
    break;

  case 66:
/* Line 1787 of yacc.c  */
#line 504 "ncgen.y"
    {
		    int i;
		    stackbase=(yyvsp[(2) - (2)].mark);
		    stacklen=listlength(stack);
		    /* process each variable in the varlist*/
	            for(i=stackbase;i<stacklen;i++) {
	                Symbol* sym = (Symbol*)listget(stack,i);
			sym->objectclass = NC_VAR;
		        if(dupobjectcheck(NC_VAR,sym)) {
                            yyerror("Duplicate variable declaration for %s",
                                    sym->name);
			} else {
		  	    sym->typ.basetype = (yyvsp[(1) - (2)].sym);
	                    addtogroup(sym);
		            listpush(vardefs,(void*)sym);
			}
		    }
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		}
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 526 "ncgen.y"
    {(yyval.mark)=listlength(stack);
                 listpush(stack,(void*)(yyvsp[(1) - (1)].sym));
		}
    break;

  case 68:
/* Line 1787 of yacc.c  */
#line 530 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(void*)(yyvsp[(3) - (3)].sym));}
    break;

  case 69:
/* Line 1787 of yacc.c  */
#line 534 "ncgen.y"
    {
		    int i;
		    Dimset dimset;
		    stacklen=listlength(stack);
		    stackbase=(yyvsp[(2) - (2)].mark);
		    count = stacklen - stackbase;
		    if(count >= NC_MAX_VAR_DIMS) {
			yyerror("%s has too many dimensions",(yyvsp[(1) - (2)].sym)->name);
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
			(yyvsp[(1) - (2)].sym)->typ.dimset = dimset;
		    }
		    (yyvsp[(1) - (2)].sym)->typ.basetype = NULL; /* not yet known*/
                    (yyvsp[(1) - (2)].sym)->objectclass=NC_VAR;
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		    }
    break;

  case 70:
/* Line 1787 of yacc.c  */
#line 560 "ncgen.y"
    {(yyval.mark)=listlength(stack);}
    break;

  case 71:
/* Line 1787 of yacc.c  */
#line 561 "ncgen.y"
    {(yyval.mark)=(yyvsp[(2) - (3)].mark);}
    break;

  case 72:
/* Line 1787 of yacc.c  */
#line 564 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[(1) - (1)].sym));}
    break;

  case 73:
/* Line 1787 of yacc.c  */
#line 566 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(void*)(yyvsp[(3) - (3)].sym));}
    break;

  case 74:
/* Line 1787 of yacc.c  */
#line 570 "ncgen.y"
    {Symbol* dimsym = (yyvsp[(1) - (1)].sym);
		dimsym->objectclass = NC_DIM;
		/* Find the actual dimension*/
		dimsym = locate(dimsym);
		if(dimsym == NULL) {
		    derror("Undefined or forward referenced dimension: %s",(yyvsp[(1) - (1)].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=dimsym;
	    }
    break;

  case 75:
/* Line 1787 of yacc.c  */
#line 584 "ncgen.y"
    {(yyval.mark)=listlength(stack);
             listpush(stack,(void*)(yyvsp[(1) - (1)].sym));
	    }
    break;

  case 76:
/* Line 1787 of yacc.c  */
#line 588 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(void*)(yyvsp[(3) - (3)].sym));}
    break;

  case 77:
/* Line 1787 of yacc.c  */
#line 593 "ncgen.y"
    {
		int i;
		Dimset dimset;
		stackbase=(yyvsp[(2) - (2)].mark);
		stacklen=listlength(stack);
		count = stacklen - stackbase;
		if(count >= NC_MAX_VAR_DIMS) {
		    yyerror("%s has too many dimensions",(yyvsp[(1) - (2)].sym)->name);
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
		    (yyvsp[(1) - (2)].sym)->typ.dimset = dimset;
		}
		(yyvsp[(1) - (2)].sym)->typ.basetype = NULL; /* not yet known*/
                (yyvsp[(1) - (2)].sym)->objectclass=NC_TYPE;
                (yyvsp[(1) - (2)].sym)->subclass=NC_FIELD;
		listsetlength(stack,stackbase);/* remove stack nodes*/
		(yyval.sym) = (yyvsp[(1) - (2)].sym);
	    }
    break;

  case 78:
/* Line 1787 of yacc.c  */
#line 621 "ncgen.y"
    {(yyval.mark)=listlength(stack);}
    break;

  case 79:
/* Line 1787 of yacc.c  */
#line 622 "ncgen.y"
    {(yyval.mark)=(yyvsp[(2) - (3)].mark);}
    break;

  case 80:
/* Line 1787 of yacc.c  */
#line 626 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(void*)(yyvsp[(1) - (1)].sym));}
    break;

  case 81:
/* Line 1787 of yacc.c  */
#line 628 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(void*)(yyvsp[(3) - (3)].sym));}
    break;

  case 82:
/* Line 1787 of yacc.c  */
#line 633 "ncgen.y"
    {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     sprintf(anon,"const%u",uint32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = uint32_val;
	    }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 643 "ncgen.y"
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
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 663 "ncgen.y"
    {Symbol* vsym = (yyvsp[(1) - (1)].sym);
		if(vsym->objectclass != NC_VAR) {
		    derror("Undefined or forward referenced variable: %s",vsym->name);
		    YYABORT;
		}
		(yyval.sym)=vsym;
	    }
    break;

  case 85:
/* Line 1787 of yacc.c  */
#line 674 "ncgen.y"
    {Symbol* tsym = (yyvsp[(1) - (1)].sym);
		if(tsym->objectclass != NC_TYPE) {
		    derror("Undefined or forward referenced type: %s",tsym->name);
		    YYABORT;
		}
		(yyval.sym)=tsym;
	    }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 685 "ncgen.y"
    {Symbol* tvsym = (yyvsp[(1) - (1)].sym); Symbol* sym;
		/* disambiguate*/
		tvsym->objectclass = NC_VAR;
		sym = locate(tvsym);
		if(sym == NULL) {
		    tvsym->objectclass = NC_TYPE;
		    sym = locate(tvsym);
		    if(tvsym == NULL) {
		        derror("Undefined or forward referenced name: %s",(yyvsp[(1) - (1)].sym)->name);
		        YYABORT;
		    } else tvsym = sym;
		} else tvsym = sym;
		if(tvsym == NULL) {
		    derror("Undefined name: %s",(yyvsp[(1) - (1)].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=tvsym;
	    }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 703 "ncgen.y"
    {(yyval.sym)=(yyvsp[(1) - (1)].sym);}
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 710 "ncgen.y"
    {}
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 710 "ncgen.y"
    {}
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 714 "ncgen.y"
    { (yyval.sym)=makeattribute((yyvsp[(2) - (4)].sym),NULL,NULL,(yyvsp[(4) - (4)].datalist),ATTRGLOBAL);}
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 716 "ncgen.y"
    {Symbol* tsym = (yyvsp[(1) - (6)].sym); Symbol* vsym = (yyvsp[(2) - (6)].sym); Symbol* asym = (yyvsp[(4) - (6)].sym);
		if(vsym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,vsym,tsym,(yyvsp[(6) - (6)].datalist),ATTRVAR);
		} else {
		    derror("Doubly typed attribute: %s",asym->name);
		    YYABORT;
		}
	    }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 725 "ncgen.y"
    {Symbol* sym = (yyvsp[(1) - (5)].sym); Symbol* asym = (yyvsp[(3) - (5)].sym);
		if(sym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,sym,NULL,(yyvsp[(5) - (5)].datalist),ATTRVAR);
		} else if(sym->objectclass == NC_TYPE) {
		    (yyval.sym)=makeattribute(asym,NULL,sym,(yyvsp[(5) - (5)].datalist),ATTRGLOBAL);
		} else {
		    derror("Attribute prefix not a variable or type: %s",asym->name);
		    YYABORT;
		}
	    }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 736 "ncgen.y"
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)(yyvsp[(5) - (5)].datalist),0);}
    break;

  case 94:
/* Line 1787 of yacc.c  */
#line 738 "ncgen.y"
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[(2) - (6)].sym),(yyvsp[(1) - (6)].sym),(void*)(yyvsp[(6) - (6)].datalist),0);}
    break;

  case 95:
/* Line 1787 of yacc.c  */
#line 740 "ncgen.y"
    {(yyval.sym) = makespecial(_STORAGE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 742 "ncgen.y"
    {(yyval.sym) = makespecial(_CHUNKSIZES_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)(yyvsp[(5) - (5)].datalist),0);}
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 744 "ncgen.y"
    {(yyval.sym) = makespecial(_FLETCHER32_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 98:
/* Line 1787 of yacc.c  */
#line 746 "ncgen.y"
    {(yyval.sym) = makespecial(_DEFLATE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 748 "ncgen.y"
    {(yyval.sym) = makespecial(_SHUFFLE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 750 "ncgen.y"
    {(yyval.sym) = makespecial(_ENDIAN_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 752 "ncgen.y"
    {(yyval.sym) = makespecial(_NOFILL_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);}
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 754 "ncgen.y"
    {(yyval.sym) = makespecial(_FORMAT_FLAG,NULL,NULL,(void*)&(yyvsp[(4) - (4)].constant),1);}
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 759 "ncgen.y"
    {
	        (yyval.sym)=(yyvsp[(1) - (1)].sym);
                (yyvsp[(1) - (1)].sym)->is_ref=1;
                setpathcurrent((yyvsp[(1) - (1)].sym));
	    }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 765 "ncgen.y"
    {
	        (yyval.sym)=(yyvsp[(1) - (1)].sym);
                (yyvsp[(1) - (1)].sym)->is_ref=1;
                (yyvsp[(1) - (1)].sym)->is_prefixed=1;
	        /* path is set in ncgen.l*/
	    }
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 774 "ncgen.y"
    {}
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 775 "ncgen.y"
    {}
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 783 "ncgen.y"
    {(yyvsp[(1) - (3)].sym)->data = (yyvsp[(3) - (3)].datalist);}
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 786 "ncgen.y"
    {(yyval.datalist) = (yyvsp[(1) - (1)].datalist);}
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 787 "ncgen.y"
    {(yyval.datalist) = (yyvsp[(1) - (1)].datalist);}
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 791 "ncgen.y"
    {(yyval.datalist) = builddatalist(0);}
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 795 "ncgen.y"
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[(1) - (1)].constant)));}
    break;

  case 115:
/* Line 1787 of yacc.c  */
#line 797 "ncgen.y"
    {datalistextend((yyvsp[(1) - (3)].datalist),&((yyvsp[(3) - (3)].constant))); (yyval.datalist)=(yyvsp[(1) - (3)].datalist);}
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 801 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);}
    break;

  case 117:
/* Line 1787 of yacc.c  */
#line 802 "ncgen.y"
    {(yyval.constant)=builddatasublist((yyvsp[(2) - (3)].datalist));}
    break;

  case 118:
/* Line 1787 of yacc.c  */
#line 806 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);}
    break;

  case 119:
/* Line 1787 of yacc.c  */
#line 807 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_OPAQUE);}
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 808 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_FILLVALUE);}
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 809 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_NIL);}
    break;

  case 122:
/* Line 1787 of yacc.c  */
#line 810 "ncgen.y"
    {(yyval.constant)=makeenumconst((yyvsp[(1) - (1)].sym));}
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 815 "ncgen.y"
    {(yyval.constant)=evaluate((yyvsp[(1) - (4)].sym),(yyvsp[(3) - (4)].datalist));}
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 820 "ncgen.y"
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[(1) - (1)].constant)));}
    break;

  case 126:
/* Line 1787 of yacc.c  */
#line 822 "ncgen.y"
    {datalistextend((yyvsp[(1) - (3)].datalist),&((yyvsp[(3) - (3)].constant))); (yyval.datalist)=(yyvsp[(1) - (3)].datalist);}
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 826 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_CHAR);}
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 827 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_BYTE);}
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 828 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_SHORT);}
    break;

  case 130:
/* Line 1787 of yacc.c  */
#line 829 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT);}
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 830 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT64);}
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 831 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UBYTE);}
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 832 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_USHORT);}
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 833 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT);}
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 834 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT64);}
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 835 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_FLOAT);}
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 836 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_DOUBLE);}
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 837 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_STRING);}
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 841 "ncgen.y"
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[(1) - (1)].constant)));}
    break;

  case 140:
/* Line 1787 of yacc.c  */
#line 842 "ncgen.y"
    {(yyval.datalist)=(yyvsp[(1) - (3)].datalist); datalistextend((yyvsp[(1) - (3)].datalist),&((yyvsp[(3) - (3)].constant)));}
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 847 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT);}
    break;

  case 142:
/* Line 1787 of yacc.c  */
#line 849 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT);}
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 851 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT64);}
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 853 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT64);}
    break;

  case 145:
/* Line 1787 of yacc.c  */
#line 857 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_STRING);}
    break;

  case 146:
/* Line 1787 of yacc.c  */
#line 861 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);}
    break;

  case 147:
/* Line 1787 of yacc.c  */
#line 862 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);}
    break;

  case 148:
/* Line 1787 of yacc.c  */
#line 868 "ncgen.y"
    {(yyval.sym)=(yyvsp[(1) - (1)].sym);}
    break;


/* Line 1787 of yacc.c  */
#line 2816 "ncgen.tab.c"
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

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

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
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2050 of yacc.c  */
#line 871 "ncgen.y"


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
#include "ncgenyy.c"

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
    createrootgroup();
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
createrootgroup(void)
{
    Symbol* gsym = install(ROOTGROUPNAME);
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
makeenumconst(Symbol* econst)
{
    NCConstant con;
    markcdf4("Enum type");
    consttype = NC_ENUM;
    con.nctype = NC_ECONST;
    con.lineno = lineno;
    con.filled = 0;
    /* fix up econst to be a ref to an econst*/
    econst->objectclass = NC_TYPE;
    econst->subclass = NC_ECONST;
    {
	Symbol* defsym;
	defsym = locate(econst);
	if(defsym == NULL)
	    derror("Undefined or forward referenced enum constant: %s",econst->name);
	econst = defsym;
    }
    con.value.enumv = econst;
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
	if(!sym->is_ref && sym->objectclass == objectclass
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

char*
specialname(int flag)
{
    switch (flag) {
    case _FILLVALUE_FLAG: return "_FillValue";
    case _FORMAT_FLAG: return "_Format";
    case _STORAGE_FLAG: return "_Storage";
    case _CHUNKSIZES_FLAG: return "_ChunkSizes";
    case _FLETCHER32_FLAG: return "_Fletcher32";
    case _DEFLATE_FLAG: return "_DeflateLevel";
    case _SHUFFLE_FLAG: return "_Shuffle";
    case _ENDIAN_FLAG: return "_Endianness";
    case _NOFILL_FLAG: return "_NoFill";
    default: break;
    }
    return "<unknown>";
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
    Specialdata* special = (Specialdata*)malloc(sizeof(Specialdata));
    NCConstant iconst;
    int tf = 0;
    char* sdata = NULL;
    int idata =  -1;

    special->flags = 0;

    specials_flag += (tag == _FILLVALUE_FLAG ? 0 : 1);

    if(isconst) {
	con = (NCConstant*)data;
	list = builddatalist(1);
        dlappend(list,(NCConstant*)data);
    } else {
        list = (Datalist*)data;
        con = (NCConstant*)list->data;
    }

    if(tag == _FORMAT && vsym != NULL) {
	derror("_Format: must be global attribute");
	vsym = NULL;
    }

    switch (tag) {
    case _FLETCHER32_FLAG:
    case _SHUFFLE_FLAG:
    case _NOFILL_FLAG:
	iconst.nctype = (con->nctype == NC_STRING?NC_STRING:NC_INT);
	convert1(con,&iconst);
	tf = truefalse(&iconst,tag);
	break;
    case _FORMAT_FLAG:
    case _STORAGE_FLAG:
    case _ENDIAN_FLAG:
	iconst.nctype = NC_STRING;
	convert1(con,&iconst);
	if(iconst.nctype == NC_STRING)
	    sdata = iconst.value.stringv.stringv;
	else
	    derror("%s: illegal value",specialname(tag));
	break;
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
	/* Handle below */
	break;
    default: PANIC1("unexpected special tag: %d",tag);
    }
    
    if(vsym != NULL) special = &vsym->var.special;
    if(tag == _FORMAT_FLAG) {
	struct Kvalues* kvalue;
	int found;
	found = 0;
	/* Use the table in main.c */
        for(kvalue=legalkinds;kvalue->name;kvalue++) {
	    if(strcmp(sdata,kvalue->name) == 0) {
		format_flag = kvalue->k_flag;
		found = 1;
	        break;
	    }
	}
	if(!found)
	    derror("_Format: illegal value: %s",sdata);
    } else if(tag == _FILLVALUE_FLAG) {
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
	attr=makeattribute(install("_FillValue"),vsym,tsym,list,ATTRVAR);
    } else switch (tag) {
        case _STORAGE_FLAG:
            if(strcmp(sdata,"contiguous") == 0)
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
            if(strcmp(sdata,"little") == 0)
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
	        } else
		    derror("%s: illegal value",specialname(tag));
            }
            special->flags |= _CHUNKSIZES_FLAG;
	    /* Chunksizes => storage == chunked */
            special->flags |= _STORAGE_FLAG;
            special->_Storage = NC_CHUNKED;
            } break;
        default: PANIC1("makespecial: illegal token: %d",tag);
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
    addtogroup(asym);
    switch (kind) {
    case ATTRVAR:
        asym->att.var = vsym;
        asym->typ.basetype = tsym;
        listpush(attdefs,(void*)asym);
	break;
    case ATTRGLOBAL:
        asym->att.var = NULL; /* NULL => NC_GLOBAL*/
        asym->typ.basetype = tsym;
        listpush(gattdefs,(void*)asym);
	break;
    default: PANIC1("unexpected attribute type: %d",kind);
    }
    /* finally; check that the attribute value contains no fill values*/
    if(containsfills(data)) {
	derror("Attribute data may not contain fill values (i.e. _ ): %s",asym->name);
    }
    return asym;
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
vercheck(int ncid)
{
    char* tmsg = NULL;
    switch (ncid) {
    case NC_UBYTE: tmsg = "netCDF4 type: UBYTE"; break;
    case NC_USHORT: tmsg = "netCDF4 type: USHORT"; break;
    case NC_UINT: tmsg = "netCDF4 type: UINT"; break;
    case NC_INT64: tmsg = "netCDF4 type: INT64"; break;
    case NC_UINT64: tmsg = "netCDF4 type: UINT64"; break;
    case NC_STRING: tmsg = "netCDF4 type: STRING"; break;
    case NC_VLEN: tmsg = "netCDF4 type: VLEN"; break;
    case NC_OPAQUE: tmsg = "netCDF4 type: OPAQUE"; break;
    case NC_ENUM: tmsg = "netCDF4 type: ENUM"; break;
    case NC_COMPOUND: tmsg = "netCDF4 type: COMPOUND"; break;
    default: break;
    }
    if(tmsg != NULL) markcdf4(tmsg);
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
    NCConstant result;

    /* prepare the result */
    result.lineno = fcn->lineno;
    result.filled = 0;

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

