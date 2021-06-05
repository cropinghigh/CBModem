/* A Bison parser, made by GNU Bison 3.7.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 43 "avrparse.y"


/// Include //////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "avrasm.hh"
#include "symbol.hh"
#include "semantic.hh"
#include "utils.hh"

/// Extern ///////////////////////////////////////////////////////////////////

GLOBALS(extern);
extern char *yytext;

/// Prototypes ///////////////////////////////////////////////////////////////

int yylex();

/// yyerror //////////////////////////////////////////////////////////////////

void yyerror(char *s)
{
  s = s; // Used for debugging purposes
}

/// Start of grammar /////////////////////////////////////////////////////////


#line 103 "avrparse"

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

#include "avrparse.hh"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_STAR = 3,                       /* STAR  */
  YYSYMBOL_DIV = 4,                        /* DIV  */
  YYSYMBOL_MOD = 5,                        /* MOD  */
  YYSYMBOL_LS = 6,                         /* LS  */
  YYSYMBOL_RS = 7,                         /* RS  */
  YYSYMBOL_LE = 8,                         /* LE  */
  YYSYMBOL_GE = 9,                         /* GE  */
  YYSYMBOL_LESS = 10,                      /* LESS  */
  YYSYMBOL_GREAT = 11,                     /* GREAT  */
  YYSYMBOL_EQ = 12,                        /* EQ  */
  YYSYMBOL_NE = 13,                        /* NE  */
  YYSYMBOL_OR = 14,                        /* OR  */
  YYSYMBOL_XOR = 15,                       /* XOR  */
  YYSYMBOL_AND = 16,                       /* AND  */
  YYSYMBOL_OROR = 17,                      /* OROR  */
  YYSYMBOL_ANDAND = 18,                    /* ANDAND  */
  YYSYMBOL_LPAR = 19,                      /* LPAR  */
  YYSYMBOL_RPAR = 20,                      /* RPAR  */
  YYSYMBOL_COLON = 21,                     /* COLON  */
  YYSYMBOL_COMMA = 22,                     /* COMMA  */
  YYSYMBOL_DOT = 23,                       /* DOT  */
  YYSYMBOL_EQUAL = 24,                     /* EQUAL  */
  YYSYMBOL_PLUS = 25,                      /* PLUS  */
  YYSYMBOL_MINUS = 26,                     /* MINUS  */
  YYSYMBOL_WAVE = 27,                      /* WAVE  */
  YYSYMBOL_NOT = 28,                       /* NOT  */
  YYSYMBOL_EOL = 29,                       /* EOL  */
  YYSYMBOL_RESTART = 30,                   /* RESTART  */
  YYSYMBOL_ENDOFFILE = 31,                 /* ENDOFFILE  */
  YYSYMBOL_DEF = 32,                       /* DEF  */
  YYSYMBOL_EQU = 33,                       /* EQU  */
  YYSYMBOL_DB = 34,                        /* DB  */
  YYSYMBOL_DW = 35,                        /* DW  */
  YYSYMBOL_ORG = 36,                       /* ORG  */
  YYSYMBOL_ALIGN = 37,                     /* ALIGN  */
  YYSYMBOL_CSEG = 38,                      /* CSEG  */
  YYSYMBOL_DSEG = 39,                      /* DSEG  */
  YYSYMBOL_ESEG = 40,                      /* ESEG  */
  YYSYMBOL_BYTE = 41,                      /* BYTE  */
  YYSYMBOL_SET = 42,                       /* SET  */
  YYSYMBOL_DEVICE = 43,                    /* DEVICE  */
  YYSYMBOL_STRING = 44,                    /* STRING  */
  YYSYMBOL_MACRODEF = 45,                  /* MACRODEF  */
  YYSYMBOL_REGISTER = 46,                  /* REGISTER  */
  YYSYMBOL_REGXYZ = 47,                    /* REGXYZ  */
  YYSYMBOL_SYMBOL = 48,                    /* SYMBOL  */
  YYSYMBOL_INTEGER = 49,                   /* INTEGER  */
  YYSYMBOL_COUNTER = 50,                   /* COUNTER  */
  YYSYMBOL_FUNCTION = 51,                  /* FUNCTION  */
  YYSYMBOL_IREGREG = 52,                   /* IREGREG  */
  YYSYMBOL_IREGREGW = 53,                  /* IREGREGW  */
  YYSYMBOL_IIMMIMM = 54,                   /* IIMMIMM  */
  YYSYMBOL_IREGIMM = 55,                   /* IREGIMM  */
  YYSYMBOL_IREGIMMW = 56,                  /* IREGIMMW  */
  YYSYMBOL_IIMMREG = 57,                   /* IIMMREG  */
  YYSYMBOL_IREG = 58,                      /* IREG  */
  YYSYMBOL_IIMM = 59,                      /* IIMM  */
  YYSYMBOL_INOARGS = 60,                   /* INOARGS  */
  YYSYMBOL_IINDIRC = 61,                   /* IINDIRC  */
  YYSYMBOL_ILPM = 62,                      /* ILPM  */
  YYSYMBOL_YYACCEPT = 63,                  /* $accept  */
  YYSYMBOL_program = 64,                   /* program  */
  YYSYMBOL_e = 65,                         /* e  */
  YYSYMBOL_programlist = 66,               /* programlist  */
  YYSYMBOL_programelement = 67,            /* programelement  */
  YYSYMBOL_instruction = 68,               /* instruction  */
  YYSYMBOL_lpminst = 69,                   /* lpminst  */
  YYSYMBOL_registername = 70,              /* registername  */
  YYSYMBOL_label = 71,                     /* label  */
  YYSYMBOL_directive = 72,                 /* directive  */
  YYSYMBOL_73_1 = 73,                      /* $@1  */
  YYSYMBOL_74_2 = 74,                      /* $@2  */
  YYSYMBOL_indirectaddr = 75,              /* indirectaddr  */
  YYSYMBOL_byteexprlist = 76,              /* byteexprlist  */
  YYSYMBOL_byteelement = 77,               /* byteelement  */
  YYSYMBOL_wordexprlist = 78,              /* wordexprlist  */
  YYSYMBOL_expr = 79,                      /* expr  */
  YYSYMBOL_primary_expr = 80,              /* primary_expr  */
  YYSYMBOL_unary_expr = 81,                /* unary_expr  */
  YYSYMBOL_mult_expr = 82,                 /* mult_expr  */
  YYSYMBOL_additive_expr = 83,             /* additive_expr  */
  YYSYMBOL_shift_expr = 84,                /* shift_expr  */
  YYSYMBOL_relational_expr = 85,           /* relational_expr  */
  YYSYMBOL_equality_expr = 86,             /* equality_expr  */
  YYSYMBOL_AND_expression = 87,            /* AND_expression  */
  YYSYMBOL_exclusive_OR = 88,              /* exclusive_OR  */
  YYSYMBOL_inclusive_OR = 89,              /* inclusive_OR  */
  YYSYMBOL_logical_AND = 90,               /* logical_AND  */
  YYSYMBOL_logical_OR = 91,                /* logical_OR  */
  YYSYMBOL_composite_expr = 92             /* composite_expr  */
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
typedef yytype_uint8 yy_state_t;

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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   412

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  118
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  250

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   317


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
      55,    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   175,   175,   178,   192,   193,   206,   207,   208,   209,
     213,   251,   253,   255,   257,   259,   261,   263,   265,   267,
     269,   271,   273,   275,   278,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   304,   306,   311,   324,   325,
     347,   348,   374,   375,   376,   377,   377,   378,   378,   379,
     380,   381,   382,   383,   384,   385,   386,   387,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     420,   422,   424,   426,   445,   446,   449,   450,   453,   454,
     467,   470,   471,   472,   481,   483,   487,   488,   489,   490,
     493,   494,   496,   498,   502,   503,   505,   509,   510,   512,
     516,   517,   519,   521,   523,   527,   528,   530,   534,   535,
     539,   540,   544,   545,   549,   550,   554,   555,   559
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "STAR", "DIV", "MOD",
  "LS", "RS", "LE", "GE", "LESS", "GREAT", "EQ", "NE", "OR", "XOR", "AND",
  "OROR", "ANDAND", "LPAR", "RPAR", "COLON", "COMMA", "DOT", "EQUAL",
  "PLUS", "MINUS", "WAVE", "NOT", "EOL", "RESTART", "ENDOFFILE", "DEF",
  "EQU", "DB", "DW", "ORG", "ALIGN", "CSEG", "DSEG", "ESEG", "BYTE", "SET",
  "DEVICE", "STRING", "MACRODEF", "REGISTER", "REGXYZ", "SYMBOL",
  "INTEGER", "COUNTER", "FUNCTION", "IREGREG", "IREGREGW", "IIMMIMM",
  "IREGIMM", "IREGIMMW", "IIMMREG", "IREG", "IIMM", "INOARGS", "IINDIRC",
  "ILPM", "$accept", "program", "e", "programlist", "programelement",
  "instruction", "lpminst", "registername", "label", "directive", "$@1",
  "$@2", "indirectaddr", "byteexprlist", "byteelement", "wordexprlist",
  "expr", "primary_expr", "unary_expr", "mult_expr", "additive_expr",
  "shift_expr", "relational_expr", "equality_expr", "AND_expression",
  "exclusive_OR", "inclusive_OR", "logical_AND", "logical_OR",
  "composite_expr", YY_NULLPTR
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
     315,   316,   317
};
#endif

#define YYPACT_NINF (-172)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-48)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      72,   -20,  -172,    -3,    92,  -172,   150,  -172,   317,  -172,
    -172,  -172,    72,  -172,  -172,    21,    26,   195,   199,   214,
     233,    89,    90,    93,   250,    27,    75,  -172,    35,     9,
     266,    36,    31,   278,    52,   294,   134,    24,    51,  -172,
    -172,  -172,     6,    60,   125,    67,   130,    74,   361,   123,
     261,   127,   261,   261,   261,   261,  -172,  -172,  -172,   141,
     145,  -172,  -172,   142,   139,   160,   128,   157,   161,   179,
     183,   181,   184,  -172,   190,   191,   200,  -172,   201,  -172,
     202,  -172,   203,   206,   207,   189,   208,   209,   224,  -172,
    -172,   232,   226,   234,   235,   229,   244,   239,   248,   242,
     251,   252,   243,   253,   257,   262,   267,   273,   274,  -172,
     279,   177,   265,   285,   296,   290,  -172,   301,  -172,    47,
    -172,   261,  -172,  -172,   -14,  -172,  -172,  -172,   -10,  -172,
    -172,   304,  -172,  -172,  -172,   261,  -172,   261,   261,   261,
     261,   261,   261,   261,   261,   261,   261,   261,   261,   261,
     261,   261,   261,   261,   261,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,   261,  -172,  -172,  -172,   102,  -172,   102,
     102,  -172,   261,  -172,   261,  -172,   261,   261,  -172,   102,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,   261,   -13,   102,
    -172,   -13,   302,   303,   305,   361,  -172,   261,  -172,  -172,
     310,  -172,  -172,  -172,   142,   142,   139,   139,   160,   160,
     160,   160,   128,   128,   157,   161,   179,   183,   181,   306,
     307,   308,   309,   311,   312,   318,   319,   332,  -172,   334,
     335,   336,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     9,     0,     0,    41,     0,     5,     0,    10,
      40,     1,     0,     4,     7,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     6,
      24,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    81,    82,     0,
       0,    89,    90,    94,    97,   100,   105,   108,   110,   112,
     114,   116,   118,    80,     0,     0,     0,    51,     0,    52,
       0,    53,     0,     0,     0,     0,     0,     0,     0,    38,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    21,
       0,     0,    71,     0,     0,     0,    35,     0,    58,     0,
      59,     0,    60,    76,     0,    75,    77,    61,     0,    79,
      62,     0,    86,    87,    88,     0,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    63,    50,    64,    65,    66,
      67,    54,    68,     0,    69,    57,    25,     0,    26,     0,
       0,    30,     0,    27,     0,    28,     0,     0,    29,     0,
      32,    20,    31,    19,    33,    34,    70,    72,     0,     0,
      37,     0,     0,     0,     0,     0,    46,     0,    48,    84,
       0,    91,    92,    93,    95,    96,    98,    99,   103,   104,
     101,   102,   106,   107,   109,   111,   113,   115,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    73,     0,
       0,     0,    42,    43,    44,    74,    78,    85,    55,    11,
      13,    12,    18,    14,    16,    15,    17,    22,    23,    36
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -172,  -172,  -172,   216,    79,  -172,  -172,    -8,  -172,  -172,
    -172,  -172,  -171,  -172,   138,  -172,   -19,  -172,   -51,    32,
      33,    -4,    68,   175,   188,   215,   213,   227,  -172,  -172
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,    39,    40,    91,     8,    41,
      48,    50,   114,   124,   125,   128,   126,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      60,    75,   132,   133,   134,    83,    -2,     1,   195,     9,
      92,    96,   197,   111,   103,   196,   107,   229,    10,   198,
     231,    94,    43,    98,   101,   110,   105,    45,    84,   113,
     117,   129,    99,   131,   112,     2,    88,    97,    -3,    -3,
      -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,
     111,    -3,   115,   104,     3,    89,    93,    90,    -3,    -3,
      -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    44,
      89,   112,    90,     1,    46,    85,    86,    89,   100,    90,
     116,    89,    89,    90,    90,    13,   201,   202,   203,   118,
      76,    78,    11,   192,    80,   193,   120,    89,    89,    90,
      90,     2,   194,   122,    -3,    -3,    -3,    -3,    -3,    -3,
      -3,    -3,    -3,    -3,    -3,    -3,   200,    -3,    77,    79,
       3,    13,    81,    87,    -3,    -3,    -3,    -3,    -3,    -3,
      -3,    -3,    -3,    -3,    -3,   108,   144,   145,   146,   147,
     208,   209,   210,   211,   219,   137,   138,   139,    89,   119,
      90,     1,   127,   223,   121,   224,   130,   225,   226,   220,
     135,   221,   222,   109,   140,   141,   142,   143,   228,   148,
     149,   227,   204,   205,   136,   206,   207,   150,   236,     2,
      12,   230,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,
      -3,    -3,    -3,    -3,   151,    -3,    47,   152,     3,   153,
      49,   154,    -3,    -3,    -3,    -3,    -3,    -3,    -3,    -3,
      -3,    -3,    -3,   163,   -45,    51,   212,   213,   -47,   155,
     156,   -45,   -45,   -45,   186,   -47,   -47,   -47,    42,   157,
     158,   159,   160,    52,    74,   161,   162,   164,   165,   -45,
      53,    54,    55,   -45,   -45,   -45,   -45,   -47,   -47,   -47,
     -47,    82,    52,   166,   167,   168,   169,   170,   171,    53,
      54,    55,    56,    57,    58,    59,   172,    95,   173,    52,
     174,   175,   178,   176,   177,   179,    53,    54,    55,   102,
      52,    56,    57,    58,    59,    52,   180,    53,    54,    55,
     187,   181,    53,    54,    55,   106,   182,    52,    56,    57,
      58,    59,   183,   184,    53,    54,    55,   188,   185,    56,
      57,    58,    59,    52,    56,    57,    58,    59,   189,   190,
      53,    54,    55,   191,   199,   214,    56,    57,    58,    59,
     237,   232,   233,   235,   234,   238,   239,   240,   241,   215,
     242,   243,    56,    57,    58,    59,    14,   244,   245,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   246,    27,   247,   248,   249,   217,   216,     0,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      52,   218,     0,     0,     0,     0,     0,    53,    54,    55,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   123,     0,     0,     0,    56,
      57,    58,    59
};

static const yytype_int16 yycheck[] =
{
      19,    20,    53,    54,    55,    24,     0,     1,    22,    29,
       1,    30,    22,    26,    33,    29,    35,   188,    21,    29,
     191,    29,     1,    31,    32,     1,    34,     1,     1,    37,
      38,    50,     1,    52,    47,    29,     1,     1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      26,    45,     1,     1,    48,    46,    47,    48,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    48,
      46,    47,    48,     1,    48,    48,     1,    46,    47,    48,
      29,    46,    46,    48,    48,     6,   137,   138,   139,    29,
       1,     1,     0,    46,     1,    48,    29,    46,    46,    48,
      48,    29,   121,    29,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,   135,    45,    29,    29,
      48,    42,    29,    48,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,     1,     8,     9,    10,    11,
     144,   145,   146,   147,   163,     3,     4,     5,    46,    24,
      48,     1,    29,   172,    24,   174,    29,   176,   177,   167,
      19,   169,   170,    29,    25,    26,     6,     7,   187,    12,
      13,   179,   140,   141,    29,   142,   143,    16,   197,    29,
      30,   189,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    15,    45,     1,    14,    48,    18,
       1,    17,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    24,    19,     1,   148,   149,    19,    29,
      29,    26,    27,    28,    47,    26,    27,    28,    12,    29,
      29,    29,    29,    19,     1,    29,    29,    29,    29,    44,
      26,    27,    28,    48,    49,    50,    51,    48,    49,    50,
      51,     1,    19,    29,    22,    29,    22,    22,    29,    26,
      27,    28,    48,    49,    50,    51,    22,     1,    29,    19,
      22,    29,    29,    22,    22,    22,    26,    27,    28,     1,
      19,    48,    49,    50,    51,    19,    29,    26,    27,    28,
      25,    29,    26,    27,    28,     1,    29,    19,    48,    49,
      50,    51,    29,    29,    26,    27,    28,    22,    29,    48,
      49,    50,    51,    19,    48,    49,    50,    51,    22,    29,
      26,    27,    28,    22,    20,   150,    48,    49,    50,    51,
      20,    29,    29,   195,    29,    29,    29,    29,    29,   151,
      29,    29,    48,    49,    50,    51,    29,    29,    29,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    29,    45,    29,    29,    29,   153,   152,    -1,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      19,   154,    -1,    -1,    -1,    -1,    -1,    26,    27,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    48,
      49,    50,    51
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,    29,    48,    64,    65,    66,    67,    71,    29,
      21,     0,    30,    67,    29,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    45,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    68,
      69,    72,    66,     1,    48,     1,    48,     1,    73,     1,
      74,     1,    19,    26,    27,    28,    48,    49,    50,    51,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,     1,    79,     1,    29,     1,    29,
       1,    29,     1,    79,     1,    48,     1,    48,     1,    46,
      48,    70,     1,    47,    70,     1,    79,     1,    70,     1,
      47,    70,     1,    79,     1,    70,     1,    79,     1,    29,
       1,    26,    47,    70,    75,     1,    29,    70,    29,    24,
      29,    24,    29,    44,    76,    77,    79,    29,    78,    79,
      29,    79,    81,    81,    81,    19,    29,     3,     4,     5,
      25,    26,     6,     7,     8,     9,    10,    11,    12,    13,
      16,    15,    14,    18,    17,    29,    29,    29,    29,    29,
      29,    29,    29,    24,    29,    29,    29,    22,    29,    22,
      22,    29,    22,    29,    22,    29,    22,    22,    29,    22,
      29,    29,    29,    29,    29,    29,    47,    25,    22,    22,
      29,    22,    46,    48,    79,    22,    29,    22,    29,    20,
      79,    81,    81,    81,    82,    82,    83,    83,    84,    84,
      84,    84,    85,    85,    86,    87,    88,    89,    90,    79,
      70,    70,    70,    79,    79,    79,    79,    70,    79,    75,
      70,    75,    29,    29,    29,    77,    79,    20,    29,    29,
      29,    29,    29,    29,    29,    29,    29,    29,    29,    29
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    63,    64,    65,    66,    66,    67,    67,    67,    67,
      67,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    69,    69,    69,    70,    70,
      71,    71,    72,    72,    72,    73,    72,    74,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      75,    75,    75,    75,    76,    76,    77,    77,    78,    78,
      79,    80,    80,    80,    80,    80,    81,    81,    81,    81,
      82,    82,    82,    82,    83,    83,    83,    84,    84,    84,
      85,    85,    85,    85,    85,    86,    86,    86,    87,    87,
      88,    88,    89,    89,    90,    90,    91,    91,    92
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     0,     2,     1,     2,     2,     2,     1,
       2,     5,     5,     5,     5,     5,     5,     5,     5,     3,
       3,     2,     5,     5,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     5,     3,     1,     1,
       2,     1,     5,     5,     5,     0,     4,     0,     4,     3,
       3,     2,     2,     2,     3,     5,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     1,     2,     3,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     1,     3,     4,     2,     2,     2,     1,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1
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
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
  case 6: /* programelement: label instruction  */
#line 206 "avrparse.y"
                                     { tolog(); }
#line 1379 "avrparse"
    break;

  case 7: /* programelement: label EOL  */
#line 207 "avrparse.y"
                                     { tolog(); }
#line 1385 "avrparse"
    break;

  case 8: /* programelement: label directive  */
#line 208 "avrparse.y"
                                     { tolog(); }
#line 1391 "avrparse"
    break;

  case 9: /* programelement: EOL  */
#line 209 "avrparse.y"
                                     { tolog(); }
#line 1397 "avrparse"
    break;

  case 10: /* programelement: error EOL  */
#line 213 "avrparse.y"
                                     { error(E_UNKNOWN_OPCODE); tolog();  }
#line 1403 "avrparse"
    break;

  case 11: /* instruction: IREGREG registername COMMA registername EOL  */
#line 252 "avrparse.y"
                  {genRegReg((yyvsp[-4].opcode), (yyvsp[-3].regid), (yyvsp[-1].regid));}
#line 1409 "avrparse"
    break;

  case 12: /* instruction: IREGREGW registername COMMA registername EOL  */
#line 254 "avrparse.y"
                  {genRegReg((yyvsp[-4].opcode), (yyvsp[-3].regid), (yyvsp[-1].regid));}
#line 1415 "avrparse"
    break;

  case 13: /* instruction: IREGREGW REGXYZ COMMA registername EOL  */
#line 256 "avrparse.y"
                  {genRegReg((yyvsp[-4].opcode), (yyvsp[-3].regid), (yyvsp[-1].regid));}
#line 1421 "avrparse"
    break;

  case 14: /* instruction: IREGIMM registername COMMA expr EOL  */
#line 258 "avrparse.y"
                  {genRegImm((yyvsp[-4].opcode), (yyvsp[-3].regid), &(yyvsp[-1].val));}
#line 1427 "avrparse"
    break;

  case 15: /* instruction: IREGIMMW registername COMMA expr EOL  */
#line 260 "avrparse.y"
                  {genRegImm((yyvsp[-4].opcode), (yyvsp[-3].regid), &(yyvsp[-1].val));}
#line 1433 "avrparse"
    break;

  case 16: /* instruction: IREGIMMW REGXYZ COMMA expr EOL  */
#line 262 "avrparse.y"
                  {genRegImm((yyvsp[-4].opcode), (yyvsp[-3].regid), &(yyvsp[-1].val));}
#line 1439 "avrparse"
    break;

  case 17: /* instruction: IIMMREG expr COMMA registername EOL  */
#line 264 "avrparse.y"
                  {genImmReg((yyvsp[-4].opcode), &(yyvsp[-3].val), (yyvsp[-1].regid));}
#line 1445 "avrparse"
    break;

  case 18: /* instruction: IIMMIMM expr COMMA expr EOL  */
#line 266 "avrparse.y"
                  {genImmImm((yyvsp[-4].opcode), &(yyvsp[-3].val), &(yyvsp[-1].val));}
#line 1451 "avrparse"
    break;

  case 19: /* instruction: IIMM expr EOL  */
#line 268 "avrparse.y"
                  {genImmedi((yyvsp[-2].opcode), &(yyvsp[-1].val));}
#line 1457 "avrparse"
    break;

  case 20: /* instruction: IREG registername EOL  */
#line 270 "avrparse.y"
                  {genRegist((yyvsp[-2].opcode), (yyvsp[-1].regid));}
#line 1463 "avrparse"
    break;

  case 21: /* instruction: INOARGS EOL  */
#line 272 "avrparse.y"
                  {genNoargs((yyvsp[-1].opcode));}
#line 1469 "avrparse"
    break;

  case 22: /* instruction: IINDIRC registername COMMA indirectaddr EOL  */
#line 274 "avrparse.y"
                  {genIndirc((yyvsp[-4].opcode), &(yyvsp[-1].indi), (yyvsp[-3].regid),TRUE);}
#line 1475 "avrparse"
    break;

  case 23: /* instruction: IINDIRC indirectaddr COMMA registername EOL  */
#line 276 "avrparse.y"
                  {genIndirc((yyvsp[-4].opcode), &(yyvsp[-3].indi), (yyvsp[-1].regid));}
#line 1481 "avrparse"
    break;

  case 25: /* instruction: IREGREG error EOL  */
#line 282 "avrparse.y"
                                     { error(E_REGISTER_EXPECTED);  }
#line 1487 "avrparse"
    break;

  case 26: /* instruction: IREGREGW error EOL  */
#line 283 "avrparse.y"
                                     { error(E_REGISTER_EXPECTED);  }
#line 1493 "avrparse"
    break;

  case 27: /* instruction: IREGIMM error EOL  */
#line 284 "avrparse.y"
                                     { error(E_INVALID_REGIMM_SPEC);    }
#line 1499 "avrparse"
    break;

  case 28: /* instruction: IREGIMMW error EOL  */
#line 285 "avrparse.y"
                                     { error(E_INVALID_REGIMM_SPEC);    }
#line 1505 "avrparse"
    break;

  case 29: /* instruction: IIMMREG error EOL  */
#line 286 "avrparse.y"
                                     { error(E_INVALID_REGIMM_SPEC);    }
#line 1511 "avrparse"
    break;

  case 30: /* instruction: IIMMIMM error EOL  */
#line 287 "avrparse.y"
                                     { error(E_INVALID_IMMEDIATE_SPEC); }
#line 1517 "avrparse"
    break;

  case 31: /* instruction: IIMM error EOL  */
#line 288 "avrparse.y"
                                     { error(E_INVALID_IMMEDIATE_SPEC); }
#line 1523 "avrparse"
    break;

  case 32: /* instruction: IREG error EOL  */
#line 289 "avrparse.y"
                                     { error(E_INVALID_REGISTER_SPEC);  }
#line 1529 "avrparse"
    break;

  case 33: /* instruction: INOARGS error EOL  */
#line 290 "avrparse.y"
                                     { error(E_NOARGS_EXPECTED_SPEC);   }
#line 1535 "avrparse"
    break;

  case 34: /* instruction: IINDIRC error EOL  */
#line 291 "avrparse.y"
                                     { error(E_INVALID_REGISTER_SPEC);  }
#line 1541 "avrparse"
    break;

  case 35: /* lpminst: ILPM EOL  */
#line 305 "avrparse.y"
                  { genLpm((yyvsp[-1].opcode), 0, NULL, FALSE); }
#line 1547 "avrparse"
    break;

  case 36: /* lpminst: ILPM registername COMMA indirectaddr EOL  */
#line 307 "avrparse.y"
                  { genLpm((yyvsp[-4].opcode), (yyvsp[-3].regid), &(yyvsp[-1].indi), TRUE); }
#line 1553 "avrparse"
    break;

  case 37: /* lpminst: ILPM error EOL  */
#line 311 "avrparse.y"
                                 { error(E_INVALID_REGISTER_SPEC); }
#line 1559 "avrparse"
    break;

  case 39: /* registername: SYMBOL  */
#line 326 "avrparse.y"
                  { if((yyvsp[0].symb)->isdefine) 
                      (yyval.regid)=(yyvsp[0].symb)->reg;
                    else
		    {
                      (yyval.regid)=-1;
                      errorin(E_INVALID_REGISTER_SPEC);
                    }
                  }
#line 1572 "avrparse"
    break;

  case 40: /* label: SYMBOL COLON  */
#line 347 "avrparse.y"
                                { doLab((yyvsp[-1].symb)); }
#line 1578 "avrparse"
    break;

  case 42: /* directive: DEF SYMBOL EQUAL REGISTER EOL  */
#line 374 "avrparse.y"
                                                  {doDef((yyvsp[-3].symb),(yyvsp[-1].regid));}
#line 1584 "avrparse"
    break;

  case 43: /* directive: DEF SYMBOL EQUAL SYMBOL EOL  */
#line 375 "avrparse.y"
                                                  {doDef((yyvsp[-3].symb),(yyvsp[-1].symb)); }
#line 1590 "avrparse"
    break;

  case 44: /* directive: EQU SYMBOL EQUAL expr EOL  */
#line 376 "avrparse.y"
                                                  {doEqu((yyvsp[-3].symb),&(yyvsp[-1].val)); }
#line 1596 "avrparse"
    break;

  case 45: /* $@1: %empty  */
#line 377 "avrparse.y"
                     {doAdb();}
#line 1602 "avrparse"
    break;

  case 46: /* directive: DB $@1 byteexprlist EOL  */
#line 377 "avrparse.y"
                                                  {/*XXX*/                   }
#line 1608 "avrparse"
    break;

  case 47: /* $@2: %empty  */
#line 378 "avrparse.y"
                     {doAdw();}
#line 1614 "avrparse"
    break;

  case 48: /* directive: DW $@2 wordexprlist EOL  */
#line 378 "avrparse.y"
                                                  {                          }
#line 1620 "avrparse"
    break;

  case 49: /* directive: ORG expr EOL  */
#line 379 "avrparse.y"
                                                  {doOrg(&(yyvsp[-1].val));          }
#line 1626 "avrparse"
    break;

  case 50: /* directive: ALIGN expr EOL  */
#line 380 "avrparse.y"
                                                  {doAlign(&(yyvsp[-1].val));        }
#line 1632 "avrparse"
    break;

  case 51: /* directive: CSEG EOL  */
#line 381 "avrparse.y"
                                                  {                          }
#line 1638 "avrparse"
    break;

  case 52: /* directive: DSEG EOL  */
#line 382 "avrparse.y"
                                                  {                          }
#line 1644 "avrparse"
    break;

  case 53: /* directive: ESEG EOL  */
#line 383 "avrparse.y"
                                                  {                          }
#line 1650 "avrparse"
    break;

  case 54: /* directive: BYTE expr EOL  */
#line 384 "avrparse.y"
                                                  {doByt(&(yyvsp[-1].val));          }
#line 1656 "avrparse"
    break;

  case 55: /* directive: SET SYMBOL EQUAL expr EOL  */
#line 385 "avrparse.y"
                                                  {doSet((yyvsp[-3].symb),&(yyvsp[-1].val)); }
#line 1662 "avrparse"
    break;

  case 56: /* directive: MACRODEF  */
#line 386 "avrparse.y"
                                                  {                          }
#line 1668 "avrparse"
    break;

  case 57: /* directive: DEVICE SYMBOL EOL  */
#line 387 "avrparse.y"
                                                  {doDev((yyvsp[-1].symb));          }
#line 1674 "avrparse"
    break;

  case 58: /* directive: DEF error EOL  */
#line 391 "avrparse.y"
                                   { error(E_EXPECTED_ID_REG);    }
#line 1680 "avrparse"
    break;

  case 59: /* directive: EQU error EOL  */
#line 392 "avrparse.y"
                                   { error(E_EXPECTED_ID_EXPR);   }
#line 1686 "avrparse"
    break;

  case 60: /* directive: DB error EOL  */
#line 393 "avrparse.y"
                                   { error(E_EXPECTED_VALLIST);   }
#line 1692 "avrparse"
    break;

  case 61: /* directive: DW error EOL  */
#line 394 "avrparse.y"
                                   { error(E_EXPECTED_VALLIST);   }
#line 1698 "avrparse"
    break;

  case 62: /* directive: ORG error EOL  */
#line 395 "avrparse.y"
                                   { error(E_EXPECTED_VAL_LABEL); }
#line 1704 "avrparse"
    break;

  case 63: /* directive: ALIGN error EOL  */
#line 396 "avrparse.y"
                                   { error(E_EXPECTED_VAL_LABEL); }
#line 1710 "avrparse"
    break;

  case 64: /* directive: CSEG error EOL  */
#line 397 "avrparse.y"
                                   { error(E_EXPECTED_NOARGS);    }
#line 1716 "avrparse"
    break;

  case 65: /* directive: DSEG error EOL  */
#line 398 "avrparse.y"
                                   { error(E_EXPECTED_NOARGS);    }
#line 1722 "avrparse"
    break;

  case 66: /* directive: ESEG error EOL  */
#line 399 "avrparse.y"
                                   { error(E_EXPECTED_NOARGS);    }
#line 1728 "avrparse"
    break;

  case 67: /* directive: BYTE error EOL  */
#line 400 "avrparse.y"
                                   { error(E_EXPECTED_VAL_LABEL); }
#line 1734 "avrparse"
    break;

  case 68: /* directive: SET error EOL  */
#line 401 "avrparse.y"
                                   { error(E_EXPECTED_ID_EXPR);   }
#line 1740 "avrparse"
    break;

  case 69: /* directive: DEVICE error EOL  */
#line 402 "avrparse.y"
                                   { error(E_EXPECTED_DEVICE);    }
#line 1746 "avrparse"
    break;

  case 70: /* indirectaddr: MINUS REGXYZ  */
#line 421 "avrparse.y"
                  {(yyval.indi).regno=(yyvsp[0].regid);(yyval.indi).plus=2;(yyval.indi).disp=0;}
#line 1752 "avrparse"
    break;

  case 71: /* indirectaddr: REGXYZ  */
#line 423 "avrparse.y"
                  {(yyval.indi).regno=(yyvsp[0].regid);(yyval.indi).plus=0;(yyval.indi).disp=0;}
#line 1758 "avrparse"
    break;

  case 72: /* indirectaddr: REGXYZ PLUS  */
#line 425 "avrparse.y"
                  {(yyval.indi).regno=(yyvsp[-1].regid);(yyval.indi).plus=1;(yyval.indi).disp=0;}
#line 1764 "avrparse"
    break;

  case 73: /* indirectaddr: REGXYZ PLUS expr  */
#line 427 "avrparse.y"
                  { (yyval.indi).regno  = (yyvsp[-2].regid);
                    (yyval.indi).plus=1;(yyval.indi).disp=1;(yyval.indi).offset=(yyvsp[0].val);}
#line 1771 "avrparse"
    break;

  case 76: /* byteelement: STRING  */
#line 449 "avrparse.y"
                                          {doAdb((yyvsp[0].string)); }
#line 1777 "avrparse"
    break;

  case 77: /* byteelement: expr  */
#line 450 "avrparse.y"
                                          {doAdb(&(yyvsp[0].val));}
#line 1783 "avrparse"
    break;

  case 78: /* wordexprlist: wordexprlist COMMA expr  */
#line 453 "avrparse.y"
                                          {doAdw(&(yyvsp[0].val));}
#line 1789 "avrparse"
    break;

  case 79: /* wordexprlist: expr  */
#line 454 "avrparse.y"
                                          {doAdw(&(yyvsp[0].val));}
#line 1795 "avrparse"
    break;

  case 83: /* primary_expr: SYMBOL  */
#line 473 "avrparse.y"
                  {(yyval.val).valid=(yyvsp[0].symb)->valid;
                   (yyval.val).value=(yyvsp[0].symb)->value
                   +(yyvsp[0].symb)->islabel*(yyvsp[0].symb)->macrolabel*
                    (yyoffset    *((yyvsp[0].symb)->segment==SEGMENT_CODE))
                   +(yyvsp[0].symb)->islabel*(yyvsp[0].symb)->macrolabel*
                    (yydataoffset*((yyvsp[0].symb)->segment==SEGMENT_DATA))
                   +(yyvsp[0].symb)->islabel*(yyvsp[0].symb)->macrolabel*
                    (yyeromoffset*((yyvsp[0].symb)->segment==SEGMENT_EEPROM));}
#line 1808 "avrparse"
    break;

  case 84: /* primary_expr: LPAR expr RPAR  */
#line 482 "avrparse.y"
                  { (yyval.val)=(yyvsp[-1].val); }
#line 1814 "avrparse"
    break;

  case 85: /* primary_expr: FUNCTION LPAR expr RPAR  */
#line 484 "avrparse.y"
                  { genFun((yyvsp[-3].func), &(yyvsp[-1].val), &(yyval.val)); }
#line 1820 "avrparse"
    break;

  case 86: /* unary_expr: MINUS unary_expr  */
#line 487 "avrparse.y"
                                   { oprUna(OP_MINUS, &(yyvsp[0].val), &(yyval.val)); }
#line 1826 "avrparse"
    break;

  case 87: /* unary_expr: WAVE unary_expr  */
#line 488 "avrparse.y"
                                   { oprUna(OP_WAVE , &(yyvsp[0].val), &(yyval.val)); }
#line 1832 "avrparse"
    break;

  case 88: /* unary_expr: NOT unary_expr  */
#line 489 "avrparse.y"
                                   { oprUna(OP_NOT  , &(yyvsp[0].val), &(yyval.val)); }
#line 1838 "avrparse"
    break;

  case 91: /* mult_expr: mult_expr STAR unary_expr  */
#line 495 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_STAR  , &(yyvsp[0].val), &(yyval.val)); }
#line 1844 "avrparse"
    break;

  case 92: /* mult_expr: mult_expr DIV unary_expr  */
#line 497 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_DIV   , &(yyvsp[0].val), &(yyval.val)); }
#line 1850 "avrparse"
    break;

  case 93: /* mult_expr: mult_expr MOD unary_expr  */
#line 499 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_MOD   , &(yyvsp[0].val), &(yyval.val)); }
#line 1856 "avrparse"
    break;

  case 95: /* additive_expr: additive_expr PLUS mult_expr  */
#line 504 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_PLUS  , &(yyvsp[0].val), &(yyval.val)); }
#line 1862 "avrparse"
    break;

  case 96: /* additive_expr: additive_expr MINUS mult_expr  */
#line 506 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_MINUS , &(yyvsp[0].val), &(yyval.val)); }
#line 1868 "avrparse"
    break;

  case 98: /* shift_expr: shift_expr LS additive_expr  */
#line 511 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_LS    , &(yyvsp[0].val), &(yyval.val)); }
#line 1874 "avrparse"
    break;

  case 99: /* shift_expr: shift_expr RS additive_expr  */
#line 513 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_RS    , &(yyvsp[0].val), &(yyval.val)); }
#line 1880 "avrparse"
    break;

  case 101: /* relational_expr: relational_expr LESS shift_expr  */
#line 518 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_LESS  , &(yyvsp[0].val), &(yyval.val)); }
#line 1886 "avrparse"
    break;

  case 102: /* relational_expr: relational_expr GREAT shift_expr  */
#line 520 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_GREAT , &(yyvsp[0].val), &(yyval.val)); }
#line 1892 "avrparse"
    break;

  case 103: /* relational_expr: relational_expr LE shift_expr  */
#line 522 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_LE    , &(yyvsp[0].val), &(yyval.val)); }
#line 1898 "avrparse"
    break;

  case 104: /* relational_expr: relational_expr GE shift_expr  */
#line 524 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_GE    , &(yyvsp[0].val), &(yyval.val)); }
#line 1904 "avrparse"
    break;

  case 106: /* equality_expr: equality_expr EQ relational_expr  */
#line 529 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_EQ    , &(yyvsp[0].val), &(yyval.val)); }
#line 1910 "avrparse"
    break;

  case 107: /* equality_expr: equality_expr NE relational_expr  */
#line 531 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_NE    , &(yyvsp[0].val), &(yyval.val)); }
#line 1916 "avrparse"
    break;

  case 109: /* AND_expression: AND_expression AND equality_expr  */
#line 536 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_AND   , &(yyvsp[0].val), &(yyval.val)); }
#line 1922 "avrparse"
    break;

  case 111: /* exclusive_OR: exclusive_OR XOR AND_expression  */
#line 541 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_XOR   , &(yyvsp[0].val), &(yyval.val)); }
#line 1928 "avrparse"
    break;

  case 113: /* inclusive_OR: inclusive_OR OR exclusive_OR  */
#line 546 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_OR    , &(yyvsp[0].val), &(yyval.val)); }
#line 1934 "avrparse"
    break;

  case 115: /* logical_AND: logical_AND ANDAND inclusive_OR  */
#line 551 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_ANDAND, &(yyvsp[0].val), &(yyval.val)); }
#line 1940 "avrparse"
    break;

  case 117: /* logical_OR: logical_OR OROR logical_AND  */
#line 556 "avrparse.y"
                  { oprBin(&(yyvsp[-2].val),OP_OROR  , &(yyvsp[0].val), &(yyval.val)); }
#line 1946 "avrparse"
    break;


#line 1950 "avrparse"

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
      yyerror (YY_("syntax error"));
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


#if !defined yyoverflow
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

  return yyresult;
}

